#include "CommandPool.h"

VK_Objects::CommandPool::CommandPool(const Device& _device, POOL_TYPE type, VkCommandPoolCreateFlags flags):device(_device)
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

	if (type == POOL_TYPE::GRAPHICS)
		createInfo.queueFamilyIndex = device.getGraphicsQueueIndex();
	else if(type == POOL_TYPE::COMPUTE)
		createInfo.queueFamilyIndex = device.getComputeQueueIndex();
	else if (type == POOL_TYPE::TRANSFER)
		createInfo.queueFamilyIndex = device.getTransferQueueIndex();

	VkResult result = vkCreateCommandPool(device.getLogicalDevice(), &createInfo, nullptr, &vk_cmdPool);

	if (result != VK_SUCCESS)std::cout << "Failed to create CommandPool\n";
}

std::unique_ptr<VK_Objects::CommandBuffer> VK_Objects::CommandPool::requestCommandBuffer(VkCommandBufferLevel level)const
{

	std::unique_ptr<CommandBuffer> cmd = 	std::make_unique<VK_Objects::CommandBuffer>(level);

	 allocateCommandBuffer(cmd->getCommandBufferHandle(), level);

	 return std::move(cmd);

}

VkCommandPool& VK_Objects::CommandPool::getPoolHanndle()
{
	return vk_cmdPool;
}

void VK_Objects::CommandPool::allocateCommandBuffer(std::vector<CommandBuffer>& cmdBuffers, VkCommandBufferLevel level)const
{

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vk_cmdPool;
	allocInfo.level = level;
	allocInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());
	
	std::vector<VkCommandBuffer> buffers;
	for (auto& buffer : cmdBuffers)buffers.push_back(buffer.getCommandBufferHandle());


	VkResult result = vkAllocateCommandBuffers(device.getLogicalDevice(), &allocInfo, buffers.data());
	if (result != VK_SUCCESS)std::cout << "    Failed to allocate CommandBuffer\n";

}

void VK_Objects::CommandPool::allocateCommandBuffer(VkCommandBuffer& cmdBuffer, VkCommandBufferLevel level) const
{

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vk_cmdPool;
	allocInfo.level = level;
	allocInfo.commandBufferCount = static_cast<uint32_t>(1);


	VkResult result = vkAllocateCommandBuffers(device.getLogicalDevice(), &allocInfo, &cmdBuffer);
	if (result != VK_SUCCESS)std::cout << "    Failed to allocate CommandBuffer\n";
}

VK_Objects::CommandPool::~CommandPool()
{
	vkDestroyCommandPool(device.getLogicalDevice(), vk_cmdPool, nullptr);
}

VK_Objects::CommandBuffer::CommandBuffer( VkCommandBufferLevel level)
{
}

VkCommandBuffer& VK_Objects::CommandBuffer::getCommandBufferHandle()
{
	// TODO: inserir instrução return aqui
	return vk_cmdBuffer;
}
