#include "RenderFrame.h"

RenderFrame::RenderFrame(const VK_Objects::Device& _device, uint32_t _index):index(_index),device(_device)
{

	VkSemaphoreCreateInfo semaphInfo{};
	semaphInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(device.getLogicalDevice(), &semaphInfo, nullptr, &vk_avaibleSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(device.getLogicalDevice(), &semaphInfo, nullptr, &vk_finishSemaphore) != VK_SUCCESS ||
		vkCreateFence(device.getLogicalDevice(), &fenceInfo, nullptr, &vk_avaibleFence))
	{

		std::cout << "Failed to create syncronization objcts\n";
	}

}

VkSemaphore& RenderFrame::getRenderFinishedSemaphore()
{
	return vk_finishSemaphore;
}

VkSemaphore& RenderFrame::getImageAvaibleSemaphore()
{
	return vk_avaibleSemaphore;
}

VkFence& RenderFrame::getFrameCountControllFence()
{
	 return vk_avaibleFence;
}

VkFence& RenderFrame::getImageStillInFlightFence()
{
	return vk_imageStillinFlightFence;
}

RenderFrame::~RenderFrame()	
{
	vkDestroySemaphore(device.getLogicalDevice(), vk_avaibleSemaphore, device.getAllocator());
	vkDestroySemaphore(device.getLogicalDevice(), vk_finishSemaphore, device.getAllocator());
	vkDestroyFence(device.getLogicalDevice(), vk_avaibleFence, device.getAllocator());
	//vkDestroyFence(device.getLogicalDevice(), vk_imageStillinFlightFence, device.getAllocator());


}

