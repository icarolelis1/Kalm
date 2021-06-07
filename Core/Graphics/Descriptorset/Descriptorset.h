#include "Graphics/Pipeline/Pipeline.h"
namespace VK_Objects {
	
	class Descriptorset {

	public: 
		
		Descriptorset(const Device *_device);
		Descriptorset(const Descriptorset& other);

		~Descriptorset();


		void  updateDescriptorset(std::vector<VkDescriptorBufferInfo>& bufferInfos,std::vector<VkDescriptorImageInfo>& imageInfos,bool dynamicUbo = 0);
		void  updateDescriptorset(int startIndex , std::vector<VkDescriptorImageInfo>& imageInfos,bool isInputAttachment = 0);


		VkDescriptorSet& getDescriptorSetHandle();

	private:

		const Device* device;
		

		VkDescriptorSet	vk_descriptorset;

	};

	using PDescriptorset = std::unique_ptr< Descriptorset>;
}