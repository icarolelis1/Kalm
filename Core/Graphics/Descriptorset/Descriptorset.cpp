#include "Descriptorset.h"

VK_Objects::Descriptorset::Descriptorset(const Device* _device) : device(_device)
{
}

VK_Objects::Descriptorset::Descriptorset(const Descriptorset& other)
{
	vk_descriptorset = other.vk_descriptorset;
	device = other.device;
}

VK_Objects::Descriptorset::~Descriptorset()
{

}

void VK_Objects::Descriptorset::updateDescriptorset(std::vector<VkDescriptorBufferInfo>& bufferInfos, std::vector<VkDescriptorImageInfo>& imageInfos, bool dynamicUbo)
{
	std::vector<VkWriteDescriptorSet> writes;
	uint32_t i = 0;

	//We first write buffers, then images.
	for (auto& bufferInfo : bufferInfos) {

		//Write only one descriptor per time;
		VkWriteDescriptorSet w{};
		w.descriptorCount = 1;
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		if (dynamicUbo)
			w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		w.dstBinding = i; i++;
		w.pBufferInfo = &bufferInfo;
		w.dstSet = vk_descriptorset;
		writes.push_back(w);

	}

	for (auto& imageInfo : imageInfos) {

		//Write only one descriptor per time;
		VkWriteDescriptorSet w{};
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.descriptorCount = 1;
		w.dstBinding = i; i++;
		w.pImageInfo = &imageInfo;
		w.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		w.dstSet = vk_descriptorset;

		writes.push_back(w);

	}

	vkUpdateDescriptorSets(device->getLogicalDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0,NULL);

}

void VK_Objects::Descriptorset::updateDescriptorset(int startIndex, std::vector<VkDescriptorImageInfo>& imageInfos, bool isInputAttachment)
{

	std::vector<VkWriteDescriptorSet> writes;
	uint32_t i = startIndex;

	
	for (auto& imageInfo : imageInfos) {

		//Write only one descriptor per time;
		VkWriteDescriptorSet w{};
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.descriptorCount = 1;
		w.dstBinding = i; i++;
		w.pImageInfo = &imageInfo;
		if(!isInputAttachment)
		w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		else {
		w.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		}
		w.dstSet = vk_descriptorset;

		writes.push_back(w);

	}
	vkUpdateDescriptorSets(device->getLogicalDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, NULL);

}

VkDescriptorSet& VK_Objects::Descriptorset::getDescriptorSetHandle()
{
	
	return vk_descriptorset;

}
