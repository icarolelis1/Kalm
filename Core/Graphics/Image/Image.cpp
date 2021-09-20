#define STB_IMAGE_IMPLEMENTATION
#include "Image.h"
#include "Graphics/RenderContext/RenderContext.h"


VK_Objects::Image::Image(const Device* _device, uint32_t Width, uint32_t Height, ImageFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageCreateFlags flags, VkImageAspectFlags aspectFlags, uint32_t arrayLayers, bool useMaxNumMips,VkSampleCountFlagBits samples):device(_device)
{
	uint32_t numMips = 1;

	if (useMaxNumMips) {

		numMips = getMaximumMips();
	}
	vk_extent.width = Width;
	vk_extent.height = Height;

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = Width;
	imageInfo.extent.height = Height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = numMips;
	imageInfo.arrayLayers = arrayLayers;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = samples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags = flags;

	if (vkCreateImage(device->getLogicalDevice(), &imageInfo, nullptr, &vk_image) != VK_SUCCESS) {
		throw std::runtime_error("    Failed to create image\n");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device->getLogicalDevice(), vk_image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = Vk_Functions::findMemoryType(device->getPhysicalDevice(), memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device->getLogicalDevice(), &allocInfo, nullptr, &vk_deviceMemory) != VK_SUCCESS) {
		throw std::runtime_error("    Failed to allocate image memory!\n");
	}

	vkBindImageMemory(device->getLogicalDevice(), vk_image, vk_deviceMemory, 0);

	//Image View Creation
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = vk_image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = numMips;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	VkResult result;
	result = vkCreateImageView(device->getLogicalDevice(), &viewInfo, nullptr, &vk_view);

	if (result != VK_SUCCESS)Utils::LOG("Failed to create imageView\n");

}

VK_Objects::Image::Image(const Device* device, uint32_t Width, uint32_t Height, VkImage& image, VkImageView& view)
{
	vk_image = image;
	vk_view = view;
}


VK_Objects::Image::Image(const VK_Objects::Device* _device, const char* path, VkFormat format, VkImageTiling tiling, VkImageCreateFlags flags, VK_Objects::CommandPool&  pool, uint32_t arrayLayers, bool useMaxNumMips):device(_device)
{

	uint32_t numMips = 1;

	

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	vk_extent.width = texWidth;
	vk_extent.height = texHeight;

	const uint32_t maxMips = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (useMaxNumMips) {

		numMips = maxMips < 10 ? maxMips : 10;
	}


	VkDeviceSize imageSize = texWidth * texHeight * 4;
	if (!pixels) {
		std::cout << path << std::endl;
		throw std::runtime_error("failed to load texture image for path : ~");
	}

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = texWidth;
	imageInfo.extent.height = texHeight;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = numMips;
	imageInfo.arrayLayers = arrayLayers;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags = flags;

	if (vkCreateImage(device->getLogicalDevice(), &imageInfo, device->getAllocator(), &vk_image) != VK_SUCCESS) {
		throw std::runtime_error("    Failed to create image\n");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device->getLogicalDevice(), vk_image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = Vk_Functions::findMemoryType(device->getPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(device->getLogicalDevice(), &allocInfo, nullptr, &vk_deviceMemory) != VK_SUCCESS) {
		throw std::runtime_error("    Failed to allocate image memory!\n");
	}

	vkBindImageMemory(device->getLogicalDevice(), vk_image, vk_deviceMemory, 0);

	//Image View Creation
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = vk_image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = numMips;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	VkResult result;
	result = vkCreateImageView(device->getLogicalDevice(), &viewInfo, nullptr, &vk_view);

	if (result != VK_SUCCESS)Utils::LOG("Failed to create imageView\n");


	VK_Objects::Buffer stagingBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingBuffer.id = "text";
	void* data;
	vkMapMemory(device->getLogicalDevice(), stagingBuffer.getMemoryHandle(), 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device->getLogicalDevice(), stagingBuffer.getMemoryHandle());

	free(pixels);
	Vk_Functions::setImageLayout(*device, pool, vk_image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 0, numMips);
	Vk_Functions::copyBufferToImage(stagingBuffer, vk_image, *device, pool, texWidth, texHeight, 0, 1);
	Vk_Functions::setImageLayout(*device, pool, vk_image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 0, numMips);

}

const VkImageView* VK_Objects::Image::getVkImageViewHandle()
{
	return &vk_view;
}

VkImage& VK_Objects::Image::getVkImageHandle()
{

	return vk_image;
}




VkExtent2D VK_Objects::Image::getExtent()
{
	return vk_extent;
}


uint32_t VK_Objects::Image::getNumberOfLayers()
{
	return numLayers;
}

void VK_Objects::Image::destroy()
{
	vkDestroyImage(device->getLogicalDevice(), vk_image, device->getAllocator());
	vkDestroyImageView(device->getLogicalDevice(), vk_view, device->getAllocator());
	vkFreeMemory(device->getLogicalDevice(), vk_deviceMemory, device->getAllocator());
}

VK_Objects::Image::~Image()
{
	destroy();
}

uint32_t VK_Objects::Image::getMaximumMips()
{
	uint32_t maxMips =  static_cast<uint32_t>(std::floor(std::log2(std::max(vk_extent.width, vk_extent.height)))) + 1;
	 maxMips =  maxMips < 10 ? maxMips : 10;
	 return maxMips;

}



namespace Vk_Functions{

	void setImageLayout(VK_Objects::Device device, VK_Objects::CommandPool& pool, VkImage& image, VkFormat format, VkImageLayout oldImageLayout, VkImageLayout newLayout, uint32_t layerCount, int baseLayer, uint32_t mipLevels)
	{
		RENDER::RenderContext r(device);
		VK_Objects::PComandBuffer cmd = r.beginCommand(pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);


		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldImageLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = baseLayer;
		barrier.subresourceRange.layerCount = layerCount;

		VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		switch (oldImageLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			barrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			break;
		}

		switch (newLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:

			barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			if (barrier.srcAccessMask == 0)
			{
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			std::cout << "Layout transition not supported";
			break;
		}

		vkCmdPipelineBarrier(
			cmd->getCommandBufferHandle(),
			sourceStage,
			destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		vkEndCommandBuffer(cmd->getCommandBufferHandle());

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd->getCommandBufferHandle();
		vkQueueSubmit(device.getQueueHandle(VK_Objects::QUEUE_TYPE::TRANSFER), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device.getQueueHandle(VK_Objects::QUEUE_TYPE::TRANSFER));

		vkFreeCommandBuffers(device.getLogicalDevice(), pool.getPoolHanndle(), 1, &cmd->getCommandBufferHandle());
		cmd.reset();
	}

	void copyBufferToImage(VK_Objects::Buffer& buffer, VkImage image, VK_Objects::Device device, VK_Objects::CommandPool& pool, uint32_t width, uint32_t height, uint32_t baseLayer, uint32_t layerCount)
	{
		RENDER::RenderContext r(device);

		VK_Objects::PComandBuffer cmd = r.beginCommand(pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = baseLayer;
		region.imageSubresource.layerCount = layerCount;
		region.imageOffset = { 0, 0, 0 };

		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(cmd->getCommandBufferHandle(), buffer.getBufferHandle(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		vkEndCommandBuffer(cmd->getCommandBufferHandle());

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd->getCommandBufferHandle();
		vkQueueSubmit(device.getQueueHandle(VK_Objects::QUEUE_TYPE::TRANSFER), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device.getQueueHandle(VK_Objects::QUEUE_TYPE::TRANSFER));

		vkFreeCommandBuffers(device.getLogicalDevice(),pool.getPoolHanndle(), 1, &cmd->getCommandBufferHandle());
	}

	void copyBufferToImage(VkBuffer& buffer, VkImage image, VkDevice device, VkCommandPool pool, uint32_t width, uint32_t height, uint32_t baseLayer, uint32_t layerCount)
	{
	}

	void generateMips(const VK_Objects::Device* device, VK_Objects::Image* image, VK_Objects::CommandPool* commandPool, uint32_t numMips)
	{
		VkCommandBuffer	 cmd = commandPool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY)->getCommandBufferHandle();

		beginCommandBuffer(cmd);

		VkExtent2D extent = image->getExtent();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image->getVkImageHandle();
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipHeight = extent.height;
		int32_t mipWidth = extent.width;	

		for (unsigned int i = 1; i < numMips; i++) {

			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(cmd,
				image->getVkImageHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image->getVkImageHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;

		}

		//Transition the last mip
		barrier.subresourceRange.baseMipLevel = numMips - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);


		Vk_Functions::endCommandBuffer(cmd);


		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;
		vkQueueSubmit(device->getQueueHandle(VK_Objects::QUEUE_TYPE::GRAPHICS) , 1, &submitInfo,VK_NULL_HANDLE);
		vkQueueWaitIdle(device->getQueueHandle(VK_Objects::QUEUE_TYPE::GRAPHICS));

		vkFreeCommandBuffers(device->getLogicalDevice(), commandPool->getPoolHanndle(), 1, &cmd);
	}

}