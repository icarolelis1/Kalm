#include "Material.h"

Engine::Material::Material(const VK_Objects::Device* _device, std::string _id, FilesPath& texturePaths, std::shared_ptr<VK_Objects::DescriptorPoolManager> pool ,VK_Objects::CommandPool * transferCommandPool, VK_Objects::CommandPool* graphicsCommandPool , int nBufferingSwapChain):
	device(_device), id(_id)
{
	//Texture resources that will be used in shader.
	VK_Objects::ShaderResource diffuseTextureResource{};
	diffuseTextureResource.binding = static_cast<uint32_t>(0);
	diffuseTextureResource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	diffuseTextureResource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

	VK_Objects::ShaderResource emissionMapResources{};
	emissionMapResources.binding = static_cast<uint32_t>(1);
	emissionMapResources.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	emissionMapResources.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

	VK_Objects::ShaderResource roughnessMapResource{};
	roughnessMapResource.binding = static_cast<uint32_t>(2);
	roughnessMapResource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	roughnessMapResource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

	VK_Objects::ShaderResource metallicnessMapResource{};
	metallicnessMapResource.binding = static_cast<uint32_t>(3);
	metallicnessMapResource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	metallicnessMapResource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

	VK_Objects::ShaderResource normalMapResource{};
	normalMapResource.binding = static_cast<uint32_t>(4);
	normalMapResource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	normalMapResource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

	std::vector<VK_Objects::ShaderResource> resourceMaterial = { diffuseTextureResource, emissionMapResources, roughnessMapResource,metallicnessMapResource, normalMapResource };

	//Create a descriptorsetLayout with all the materials textures
	std::shared_ptr<VK_Objects::DescriptorSetLayout> descLayoutMaterial = std::make_shared<VK_Objects::DescriptorSetLayout>(device, resourceMaterial);

	//Since the swapChain will use N Images, we would need n descriptorsets. One for each image.
	for (int i = 0; i < nBufferingSwapChain; i++) {

		//Allocate descriptorset for descLayout and store it inside sets variable.
		descriptorsets.push_back(pool->allocateDescriptor(descLayoutMaterial));//

	}

	//VK_Objects::Image textureImage(&device, "Assets\\Common\\castle.jpg", VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, 0, *transferPool.get());

	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	
	//Create textures
	images["DIFFUSE_TEXTURE"] =   std::make_unique<VK_Objects::Image>(device, texturePaths.diffuseMap.c_str() , format, VK_IMAGE_TILING_OPTIMAL,  0, *transferCommandPool, 1, true);
	images["EMISSION_TEXTURE"] =  std::make_unique<VK_Objects::Image>(device, texturePaths.emissionMap.c_str(), format, VK_IMAGE_TILING_OPTIMAL,  0, *transferCommandPool, 1, true);
	images["ROUGHNESS_TEXTURE"] = std::make_unique<VK_Objects::Image>(device, texturePaths.roughnessMap.c_str(), format, VK_IMAGE_TILING_OPTIMAL,   0, *transferCommandPool, 1, true);
	images["METALLIC_TEXTURE"] = std::make_unique<VK_Objects::Image>(device, texturePaths.metallicMap.c_str(), format, VK_IMAGE_TILING_OPTIMAL, 0, *transferCommandPool, 1, true);
	images["NORMAL_TEXTURE"] =    std::make_unique<VK_Objects::Image>(device, texturePaths.normalMap.c_str(), format,  VK_IMAGE_TILING_OPTIMAL,   0, *transferCommandPool, 1, true);

	Vk_Functions::setImageLayout(*device, *transferCommandPool, images["DIFFUSE_TEXTURE"]->getVkImageHandle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 0, images["DIFFUSE_TEXTURE"]->getMaximumMips());
	Vk_Functions::setImageLayout(*device, *transferCommandPool, images["EMISSION_TEXTURE"]->getVkImageHandle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 0, images["EMISSION_TEXTURE"]->getMaximumMips());
	Vk_Functions::setImageLayout(*device, *transferCommandPool, images["ROUGHNESS_TEXTURE"]->getVkImageHandle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 0, images["ROUGHNESS_TEXTURE"]->getMaximumMips());
	Vk_Functions::setImageLayout(*device, *transferCommandPool, images["METALLIC_TEXTURE"]->getVkImageHandle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 0, images["METALLIC_TEXTURE"]->getMaximumMips());
	Vk_Functions::setImageLayout(*device, *transferCommandPool, images["NORMAL_TEXTURE"]->getVkImageHandle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 0, images["NORMAL_TEXTURE"]->getMaximumMips());

	uint32_t n = images["DIFFUSE_TEXTURE"]->getMaximumMips();
	Vk_Functions::generateMips(device, images["DIFFUSE_TEXTURE"].get(), graphicsCommandPool, images["DIFFUSE_TEXTURE"]->getMaximumMips());
	Vk_Functions::generateMips(device, images["EMISSION_TEXTURE"].get(), graphicsCommandPool, images["EMISSION_TEXTURE"]->getMaximumMips());
	Vk_Functions::generateMips(device, images["ROUGHNESS_TEXTURE"].get(), graphicsCommandPool, images["ROUGHNESS_TEXTURE"]->getMaximumMips());
	Vk_Functions::generateMips(device, images["METALLIC_TEXTURE"].get(), graphicsCommandPool, images["METALLIC_TEXTURE"]->getMaximumMips());
	Vk_Functions::generateMips(device, images["NORMAL_TEXTURE"].get(), graphicsCommandPool, images["NORMAL_TEXTURE"]->getMaximumMips());


	int index = 0;
	Vk_Functions::createSampler(device, globalSampler);

	//Write the descriptorsets
	for (int i = 0; i < descriptorsets.size(); i++) {

		std::vector<VkDescriptorBufferInfo> bufferInfos;


		std::vector<VkDescriptorImageInfo> imageInfo;
		imageInfo.resize(5);

		imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[0].imageView =  *images["DIFFUSE_TEXTURE"]->getVkImageViewHandle();
		imageInfo[0].sampler = globalSampler;

		imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[1].imageView = *images["EMISSION_TEXTURE"]->getVkImageViewHandle();
		imageInfo[1].sampler = globalSampler;

		imageInfo[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[2].imageView = *images["ROUGHNESS_TEXTURE"]->getVkImageViewHandle();
		imageInfo[2].sampler = globalSampler;

		imageInfo[3].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[3].imageView = *images["METALLIC_TEXTURE"]->getVkImageViewHandle();
		imageInfo[3].sampler = globalSampler;


		imageInfo[4].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[4].imageView = *images["NORMAL_TEXTURE"]->getVkImageViewHandle();
		imageInfo[4].sampler = globalSampler;

		descriptorsets[i].updateDescriptorset(index, imageInfo);
	}
}

VkDescriptorSet& Engine::Material::getDescriptorsetAtIndex(unsigned int i)
{
	return descriptorsets[i].getDescriptorSetHandle();
}

Engine::Material::~Material()
{
	vkDestroySampler(device->getLogicalDevice(), globalSampler, device->getAllocator());

	std::unordered_map<std::string, VK_Objects::PImage>::iterator it = images.begin();

	while (it != images.end()) {

		it->second.reset();
		it++;
	}
}
