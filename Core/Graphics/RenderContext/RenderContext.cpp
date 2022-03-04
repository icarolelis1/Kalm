#include "RenderContext.h"

//This constructor consider that Images and Framebuffers are already created


RENDER::RenderContext::RenderContext(VK_Objects::Device& _device, std::shared_ptr<VK_Objects::SwapChain> _swapChain): device(_device),swapChain(_swapChain)
{

	//maxFramesInFly defines the maximum number of frames we will be working on . This can be lesser than swapChainImages

}

RENDER::RenderContext::RenderContext(VK_Objects::Device& _device):device(_device)
{
}

void RENDER::RenderContext::setPersistentCommandBuffers(std::vector<VK_Objects::PComandBuffer> cmds)
{
persistentCommandBuffers = std::move(cmds);
for (unsigned int i = 0; i < persistentCommandBuffers.size(); i++) {
		frames.push_back(std::make_unique<RenderFrame>(device, i));
	}
}

std::unique_ptr<RENDER::RenderContext> RENDER::RenderContext::getContext()
{
	return std::unique_ptr<RenderContext>(this);
}

std::unique_ptr<VK_Objects::CommandBuffer> RENDER::RenderContext::beginCommand(VK_Objects::CommandPool &pool , VkCommandBufferLevel level , VkCommandBufferUsageFlags flags)
{
	VK_Objects::PComandBuffer buffer = pool.requestCommandBuffer(level);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = flags;

	vkBeginCommandBuffer(buffer->getCommandBufferHandle(), &beginInfo);

	return (std::move(buffer));

}

void RENDER::RenderContext::finishCommand(VK_Objects::CommandBuffer& commandBuffer, VK_Objects::CommandPool& pool)
{
	VkCommandBuffer cmd = commandBuffer.getCommandBufferHandle();
	vkEndCommandBuffer(cmd);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer.getCommandBufferHandle();

	vkQueueSubmit(device.getQueueHandle(VK_Objects::QUEUE_TYPE::GRAPHICS ), 1, &submitInfo,VK_NULL_HANDLE);
	vkQueueWaitIdle(device.getQueueHandle(VK_Objects::QUEUE_TYPE::GRAPHICS));
	
	vkFreeCommandBuffers(device.getLogicalDevice(), pool.getPoolHanndle(), 1, &commandBuffer.getCommandBufferHandle());
}

uint32_t RENDER::RenderContext::draw()
{

	vkWaitForFences(device.getLogicalDevice(), 1, &frames[currentFrameIndex]->getFrameCountControllFence(), VK_FALSE, UINT64_MAX);
	uint32_t imageIndex;

	VkResult result = vkAcquireNextImageKHR(device.getLogicalDevice(), swapChain->getSwapChainHandle(), UINT64_MAX, frames[currentFrameIndex]->getImageAvaibleSemaphore() , VK_NULL_HANDLE, &imageIndex);
	if (result != VK_SUCCESS) {
		std::cout << "Failed to aquire Image\n";

		std::cout << result << std::endl;
	};
	//swapChain->update();

	if (frames[imageIndex]->getImageStillInFlightFence() != VK_NULL_HANDLE) {
		vkWaitForFences(device.getLogicalDevice(), 1, &frames[imageIndex]->getImageStillInFlightFence(), VK_TRUE, UINT64_MAX);
	}

	frames[imageIndex]->getImageStillInFlightFence() = frames[currentFrameIndex]->getFrameCountControllFence() ;

	VkSubmitInfo submitInfo = {};

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { frames[currentFrameIndex]->getImageAvaibleSemaphore() };

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;


	VkCommandBuffer cmd = { persistentCommandBuffers[imageIndex]->getCommandBufferHandle()};

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers =&cmd;
	
	VkSemaphore signalSemaphores[] = { frames[currentFrameIndex]->getRenderFinishedSemaphore() };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	
	vkResetFences(device.getLogicalDevice(), 1, &frames[currentFrameIndex]->getFrameCountControllFence());

	VkResult r = vkQueueSubmit(device.getQueueHandle(VK_Objects::QUEUE_TYPE::GRAPHICS), 1, &submitInfo, frames[currentFrameIndex]->getFrameCountControllFence());

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { swapChain->getSwapChainHandle() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(device.getQueueHandle(VK_Objects::QUEUE_TYPE::PRESENT), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			
		//recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}



	currentFrameIndex = (currentFrameIndex+ 1) % maxFramesInFly;

	return imageIndex;
}

void RENDER::RenderContext::setMaxFramesInFlight(int n)
{
	maxFramesInFly = n;
}

void RENDER::RenderContext::setBuffers(std::vector<VK_Objects::SBuffer> _mvpBuffers, std::vector<VK_Objects::SBuffer> _lightBuffer)
{
	mvpBuffers = std::move(_mvpBuffers);
	lightBuffers = std::move(_lightBuffer);
}

