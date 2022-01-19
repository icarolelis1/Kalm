#pragma once
#include "Graphics/DescriptorPool/Descriptorpool.h"
#include <Graphics/Image/Image.h>
namespace Engine {


	struct FilesPath {

		int index;
		std::string name;
		std::string diffuseMap;
		std::string emissionMap;
		std::string metallicMap;
		std::string roughnessMap;
		std::string normalMap;

	};

	class Material {


	public:

		Material(const VK_Objects::Device* device, std::string id, FilesPath& texturePaths,std::shared_ptr<VK_Objects::DescriptorPoolManager> pool , VK_Objects::CommandPool* commandPool , VK_Objects::CommandPool* graphicsCommandPool,int nBufferingSwapChains);

		VkDescriptorSet& getDescriptorsetAtIndex(unsigned int i);

		~Material();

	private:

		VkSampler globalSampler;
		const VK_Objects::Device* device;
		std::vector<VK_Objects::Descriptorset> descriptorsets;
		std::string id;
		std::unordered_map<std::string , VK_Objects::PImage> images;

	};
}