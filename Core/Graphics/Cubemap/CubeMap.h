#pragma once
#include <Graphics/Pipeline/Pipeline.h>
#include <Material/Material.h>
#include <Graphics/Image/Image.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#ifndef CUBE_MAP
#define CUBE_MAP
namespace VK_Objects {

	class CubeMap
	{
	public:

		CubeMap(const VK_Objects::Device* _device, VkFormat format, VkMemoryPropertyFlags properties ,int dim, int numMips);

		VkImage& getVkImageHandlee();
		VkImageView& getVkViewHandle();


		~CubeMap();

	private:
		VkImage vk_image;
		VkImageView vk_view;
		VkDeviceMemory vk_memory;

		const VK_Objects::Device* device;

	};


}

namespace Vk_Functions {

	void convertEquirectangularImageToCubeMap(const VK_Objects::Device* device, const char* hdriImagePath, VK_Objects::CubeMap& cupeMap, VK_Objects::CommandPool& transientPool, VK_Objects::CommandPool& graphicsPool, VK_Objects::SDescriptorPoolManager poolManager);

	void filterEnviromentMap(const VK_Objects::Device* device, VK_Objects::CubeMap& skybox , VK_Objects::CubeMap& envMap, VK_Objects::CommandPool& transientPool, VK_Objects::CommandPool& graphicsPool, VK_Objects::SDescriptorPoolManager poolManager);

	void generatBRDFLut(const VK_Objects::Device* device,VK_Objects::Image& brdfl, VK_Objects::CommandPool& transientPool, VK_Objects::CommandPool& graphicsPool, VK_Objects::SDescriptorPoolManager poolManager);

	VK_Objects::PBuffer createCubeVertexBuffer(const VK_Objects::Device* device, VK_Objects::CommandPool* transientPool);

}

#endif