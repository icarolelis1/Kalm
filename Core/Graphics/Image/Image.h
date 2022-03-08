#pragma once
#include "stb_image.h"
#include "..\GraphicsUtil\GraphicsUtility.h"
#include "Graphics/CommandPool/CommandPool.h"

#include <memory>
#ifndef IMAGE
#define IMAGE
namespace VK_Objects {

		class Image {
		public:

			Image(const Device* device, uint32_t Width, uint32_t Height, ImageFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageCreateFlags flags, VkImageAspectFlags aspectFlags, uint32_t arrayLayers = 1, bool useMaxNumMips = 0, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

			Image(const Device* device, uint32_t Width, uint32_t Height, VkImage& image, VkImageView& view);

			Image(const VK_Objects::Image& other) = delete;;

			//Constructor for textures
			Image(const VK_Objects::Device* device, const char* path, VkFormat format, VkImageTiling tiling, VkImageCreateFlags flags, VK_Objects::CommandPool& pool, uint32_t arrayLayers=1, bool useMaxNumMips=0);
		
			const VkImageView* getVkImageViewHandle();

			VkImage& getVkImageHandle();

			VkExtent2D getExtent();

			uint32_t getNumberOfLayers();

			void destroy();

			uint32_t getMaximumMips();

			~Image();

		private:

			uint32_t numLayers;
			VkExtent2D vk_extent;
			const Device* device;
			VkImage vk_image;
			VkImageView vk_view;
			VkDeviceMemory vk_deviceMemory;

	};

		using PImage = std::unique_ptr<VK_Objects::Image>;
}


namespace Vk_Functions {

	void setImageLayout(VK_Objects::Device device, VK_Objects::CommandPool& pool, VkImage& image, VkFormat format, VkImageLayout oldImageLayout, VkImageLayout newLayout, uint32_t layerCount = 1, int baseLayer = 0, uint32_t mipLevels = 1);

	void copyBufferToImage(VK_Objects::Buffer& buffer, VkImage image, VK_Objects::Device device, VK_Objects::CommandPool* pool, uint32_t width, uint32_t height, uint32_t baseLayer = 0, uint32_t layerCount = 1);

	void copyBufferToImage(VkBuffer& buffer, VkImage image, VkDevice device, VkCommandPool pool, uint32_t width, uint32_t height, uint32_t baseLayer = 0, uint32_t layerCount = 1);

	void generateMips(const VK_Objects::Device* device,VK_Objects::Image *image, VK_Objects::CommandPool* commandPool, uint32_t numMips);
}
#endif;

