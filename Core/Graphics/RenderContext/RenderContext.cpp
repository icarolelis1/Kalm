#include "RenderContext.h"

//This constructor consider that Images and Framebuffers are already created


RENDER::RenderContext::RenderContext(VK_Objects::Device& _device, std::shared_ptr<VK_Objects::SwapChain> _swapChain): device(_device),swapChain(_swapChain)
{

	//maxFramesInFly defines the maximum number of frames we will be working on . This can be lesser than swapChainImages

}

RENDER::RenderContext::RenderContext(VK_Objects::Device& _device):device(_device)
{
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


void RENDER::RenderContext::setNumberOfFrames(int n)
{
	maxFramesInFly = n;
	for(int i=0;i<n;i++)
	frames.emplace_back(std::move(std::make_unique<RenderFrame>(device, i)));

}

