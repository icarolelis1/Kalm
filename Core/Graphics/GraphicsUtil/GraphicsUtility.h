#pragma once
#include  <Graphics/CommandPool/CommandPool.h>

namespace Vk_Functions {

		void createInstance(VK_Objects::Instance& instance, VkInstanceCreateInfo createInfo);

		void createRenderImageViews(VkDevice logicalDevice, VK_Objects::RenderImages& images, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType  viewType = VK_IMAGE_VIEW_TYPE_2D, uint32_t layerCount = 1, uint32_t numMips = 1);

		void createRenderpass(const VK_Objects::Device* device, VK_Objects::Renderpass& renderpass);

		uint32_t findMemoryType(const VkPhysicalDevice& device, uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void beginCommandBuffer(VkCommandBuffer& cmd);

		void endCommandBuffer(VkCommandBuffer& cmd);

		void createSampler(const VK_Objects::Device*,VkSampler &sampler);

		void copyBuffer(VkCommandBuffer cmd, VK_Objects::Buffer& src, VK_Objects::Buffer& dest, VkDeviceSize size, VkQueue queue);

		
		/*
		void setImageLayout(VK_Objects::Device device, VK_Objects::CommandPool& pool, VkImage& image, VkFormat format, VkImageLayout oldImageLayout, VkImageLayout newLayout, uint32_t layerCount = 1, int baseLayer = 0, uint32_t mipLevels = 1);

		void copyBufferToImage(VK_Objects::Buffer& buffer, VkImage image, VK_Objects::Device device, VK_Objects::CommandPool& pool, uint32_t width, uint32_t height, uint32_t baseLayer = 0, uint32_t layerCount =1);
		*/
}
