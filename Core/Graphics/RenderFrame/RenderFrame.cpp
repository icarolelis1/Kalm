#include "RenderFrame.h"

RenderFrame::RenderFrame(const VK_Objects::Device& _device, uint32_t _index):index(_index),device(_device)
{

	commandPools.push_back(std::make_shared<VK_Objects::CommandPool>(device, VK_Objects::POOL_TYPE::GRAPHICS, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
	commandPools.push_back(std::make_shared<VK_Objects::CommandPool>(device, VK_Objects::POOL_TYPE::GRAPHICS, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
	commandPools.push_back(std::make_shared<VK_Objects::CommandPool>(device, VK_Objects::POOL_TYPE::GRAPHICS, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

	commands.resize(3);
	secondaryCommands.resize(2);
	commandPools[0]->allocateCommandBuffer(commands[0].getCommandBufferHandle(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	commandPools[1]->allocateCommandBuffer(secondaryCommands[0].getCommandBufferHandle(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	commandPools[2]->allocateCommandBuffer(secondaryCommands[1].getCommandBufferHandle(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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

VK_Objects::CommandBuffer& RenderFrame::getCommandBuffer()
{
	return commands[0];
}
std::vector<VK_Objects::CommandBuffer>& RenderFrame::getCommandSecondaryCommandBuffer()
{
	return secondaryCommands;
}

VkCommandBuffer& RenderFrame::getCommandBufferHandler()
{
	return commands[0].getCommandBufferHandle();
}

RenderFrame::~RenderFrame()	
{
	//for (int i = 0; i < commandPools.size(); i++)
	//	vkDestroyCommandPool(device.getLogicalDevice(), commandPools[i]->getPoolHanndle(), device.getAllocator());

	vkDestroySemaphore(device.getLogicalDevice(), vk_avaibleSemaphore, device.getAllocator());
	vkDestroySemaphore(device.getLogicalDevice(), vk_finishSemaphore, device.getAllocator());
	vkDestroyFence(device.getLogicalDevice(), vk_avaibleFence, device.getAllocator());


}

