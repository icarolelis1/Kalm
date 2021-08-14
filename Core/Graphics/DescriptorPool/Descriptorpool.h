#include "Graphics/Descriptorset/Descriptorset.h"

#include <map>
#include <memory>
#define MAX_SETS 30

namespace VK_Objects {
	
	class DescriptorPool {

	public:
		DescriptorPool( const Device* _device,DescriptorSetLayout& _descriptorLayout );

		VkDescriptorPool& getPoolHandle();

		bool allocateDescriptorSet(VkDescriptorSet& set, DescriptorSetLayout& _descriptorLayout);

		~DescriptorPool();

	private:
		const Device* device;

		uint32_t allocatedSets = 0;

		VkDescriptorPool vk_pool;
			
		DescriptorSetLayout& descriptorLayout;


	};

	class DescriptorPoolManager {

	public:
		
		DescriptorPoolManager(const VK_Objects::Device* _device);

		Descriptorset allocateDescriptor(SDescriptorsetLayout _descriptorLayout);

		~DescriptorPoolManager();

		void initiatePool();
	
		VkDescriptorPool getDescriptorPoolHandle();

	private:
		const Device* device;
		std::vector<std::unique_ptr<DescriptorPool>> pools;
		uint32_t pool_index = 0;

	};

	using PDescriptorPoolManager = std::unique_ptr< DescriptorPoolManager>;
	using SDescriptorPoolManager = std::shared_ptr< DescriptorPoolManager>;

}