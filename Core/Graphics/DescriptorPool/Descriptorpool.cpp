#include "Descriptorpool.h"



VK_Objects::DescriptorPool::DescriptorPool(const Device* _device,  DescriptorSetLayout& _descriptorLayout)
: device(_device),descriptorLayout(_descriptorLayout)
{
	//vkCreateDescriptorPool(device->getLogicalDevice(),)
}

VkDescriptorPool& VK_Objects::DescriptorPool::getPoolHandle()
{
	return vk_pool;
}

bool  VK_Objects::DescriptorPool::allocateDescriptorSet( VkDescriptorSet &set, DescriptorSetLayout& _descriptorLayout)
{
	VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };

	allocInfo.descriptorPool = vk_pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &_descriptorLayout.getDescriptorLayoutHandle();


	auto result = vkAllocateDescriptorSets(device->getLogicalDevice(), &allocInfo, &set);

	if (result == VK_SUCCESS) {

		allocatedSets++;
		return 1;
	}

	return 0;
}

VK_Objects::DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(device->getLogicalDevice(), vk_pool, device->getAllocator());
}

VK_Objects::DescriptorPoolManager::DescriptorPoolManager(const VK_Objects::Device* _device):device(_device)
{
}

VK_Objects::Descriptorset VK_Objects::DescriptorPoolManager::allocateDescriptor(SDescriptorsetLayout _descriptorLayout )
{
	
	//Create new Pool
	if (static_cast<uint32_t>(pools.size()==pool_index)) {
		 
		std::vector<VkDescriptorPoolSize> pool_sizes;

		auto bindings = _descriptorLayout->getBindings();
		


		for (auto& binding : bindings) {
			
			pool_sizes.push_back({ binding.descriptorType,binding.descriptorCount * MAX_SETS });

		}

		VkDescriptorPoolCreateInfo create_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };

		// We do not set FREE_DESCRIPTOR_SET_BIT as we do not need to free individual descriptor sets
		create_info.flags = 0;
		create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size())	;
		create_info.pPoolSizes = pool_sizes.data();
		create_info.maxSets = MAX_SETS;
		
		pools.push_back(std::make_unique<DescriptorPool>(device, *_descriptorLayout.get()));

		std::cout << "\n - Created Descriptorpool number : : " << pools.size() << std::endl;
		std::cout << "\n";

		auto result = vkCreateDescriptorPool(device->getLogicalDevice(), &create_info, device->getAllocator(), &pools[pool_index]->getPoolHandle());
		
		if (result != VK_SUCCESS) {

			std::cout << "Failed to create DescriptorPool\n";
		}
	}

	// Allocate a new descriptor set from the current pool
	Descriptorset dset(device);

	if (pools[pool_index]->allocateDescriptorSet(dset.getDescriptorSetHandle(), *_descriptorLayout.get())) {

		return dset;
	}
	
	else {

		pool_index++;
		allocateDescriptor(_descriptorLayout);

	}

}

VK_Objects::DescriptorPoolManager::~DescriptorPoolManager()
{

	for (auto& pool : pools)pool.reset();

}

