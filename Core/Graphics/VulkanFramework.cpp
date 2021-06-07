#include "VulkanFramework.h"
#include "Image\Image.h"



namespace VK_Objects {

	std::vector<VkImage> SwapChain::getImages()
	{
		return swapChainImages.vk_images;
	}
	std::vector<VkImageView> &SwapChain::getViews()
	{
		return swapChainImages.vk_imageViews;
	}
	ImageFormat SwapChain::getSwapchainFormat()
	{
		return properties.format.format;
	}
	VkExtent2D SwapChain::getExtent()
	{
		return properties.extent;
	}
	uint32_t SwapChain::getNumberOfImages()
	{
		return static_cast<uint32_t>(swapChainImages.vk_images.size());
	}
	VkFormat SwapChain::getFormat()
	{
		return properties.format.format;
	}
	void SwapChain::createSwapchainViews(Device device) {

		//VK_Functions::createRenderImageViews(device.getLogicalDevice(), swapChainImages, properties.format.format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1, 1);
	
	 size_t n = swapChainImages.vk_images.size();
		swapChainImages.vk_imageViews.resize(n);

		for (size_t i = 0; i < n; i++) {

			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = swapChainImages.vk_images[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = properties.format.format;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;
			VkResult result;
			result = vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &swapChainImages.vk_imageViews[i]);
			if (result != VK_SUCCESS) {
				throw std::runtime_error("Failed to create SwapChain View\n");
			}
		}

	}

	void Instance::destroy() {

		std::cout << "    Instance Destroyed\n";
		vkDestroyInstance(vk_Instance, nullptr);
	}

	Renderpass::Renderpass(const Device* _device ,const char* _key, VkExtent2D _extent):key(_key),device(_device),extent(_extent)
	{
	}

	void Renderpass::beginRenderPass(VkCommandBuffer& commandBuffer, VkFramebuffer& frameBuffer, VkSubpassContents contents)
	{

		
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass =vk_renderpass;
		info.framebuffer = frameBuffer;
		info.renderArea.extent.width = extent.width;
		info.renderArea.extent.height = extent.height;
		info.clearValueCount = static_cast<uint32_t>(clearValues.size());

		info.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &info, contents);

	}

	void Renderpass::endRenderPass(VkCommandBuffer cmd)
	{
		vkCmdEndRenderPass(cmd);
		

	}



	const char* Renderpass::getKey()
	{
		return key;
	}

	void Renderpass::destroyRenderpass(  const VK_Objects::Device& device)
	{
		vkDestroyRenderPass(device.getLogicalDevice(), vk_renderpass, nullptr);

	}

	Renderpass::~Renderpass()
	{
		vkDestroyRenderPass(device->getLogicalDevice(), vk_renderpass, nullptr);

	}
	
	const VkQueue& Device::getQueueHandle(QUEUE_TYPE type) const
	{
		switch (type) {
		case QUEUE_TYPE::GRAPHICS:
			return graphicsQueue; break;


		case QUEUE_TYPE::PRESENT:
			return presentationQueue; break;

		case QUEUE_TYPE::TRANSFER:
		return transferQueue; break;

		}
	}
	VkFormat Device::getSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)const
	{
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(vk_physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}

	Buffer::Buffer(const Device* _device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties):device(_device)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device->getLogicalDevice(), &bufferInfo, nullptr, &vk_buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device->getLogicalDevice(), vk_buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device->getLogicalDevice(), &allocInfo, nullptr, &vk_deviceMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(	device->getLogicalDevice(), vk_buffer, vk_deviceMemory, 0);
	}


	uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(device->getPhysicalDevice(), &memoryProperties);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
	}

	bool Buffer::isMapped()
	{
		return mapped;
	}

	VkBuffer& Buffer::getBufferHandle()
	{
		return vk_buffer;
	}

	VkDeviceMemory& Buffer::getMemoryHandle()
	{
		// TODO: inserir instrução return aqui
		return vk_deviceMemory;
	}

	Buffer::~Buffer()
	{
		id;
		vkDestroyBuffer(device->getLogicalDevice(), vk_buffer, device->getAllocator());
		vkFreeMemory(device->getLogicalDevice(), vk_deviceMemory, device->getAllocator());
	}

	void Buffer::setMapped(bool m)
	{
		mapped = m;
	}

}