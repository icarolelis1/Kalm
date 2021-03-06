#include <unordered_map>
#include "Graphics/VulkanFramework.h"
#include "Graphics/VkInitializer.h"
#include <functional>

#ifndef PIPELINE
#define PIPELINE
namespace VK_Objects {


	enum class ATRIBUTES
	{
		NONE,
		FLOAT,
		VEC4,
		VEC4SI,
		VEC2,
		VEC3
	};
	
	enum class ShaderResourceType {
		IMAGE_SAMPLER,
		UNIFORM_BUFFER,
		PUSH_CONSTANT,
		INPUT_ATTACHMENT,
		IMAGE_STORAGE,
		UNIFORM_DYNAMIC

	};

	struct ShaderResource {

		ShaderResourceType type;

		uint32_t binding;

		uint32_t size;

		VkShaderStageFlags stages;


	};

	struct VertexLayout {

	public:

		std::vector<std::vector<ATRIBUTES>> atributes;

		VertexLayout(std::vector<std::vector<ATRIBUTES> >atribs_, std::vector<uint32_t>  vertexOffsets, uint32_t vertexBindingCount = 1) :numberOfBindings(vertexBindingCount) {
			atributes.resize(vertexBindingCount);
			atributes = (atribs_);
		}
		uint32_t numberOfBindings;

		uint32_t getVertexStride(uint32_t binding) {

			uint32_t res = 0;

			uint32_t firstOffset = 0;
			for (auto& component : atributes[binding])
			{
				switch (component)
				{
				case ATRIBUTES::VEC2:
					res += 2 * sizeof(float);
					break;
				case ATRIBUTES::VEC4:
					res += 4 * sizeof(float);
					break;
				case ATRIBUTES::VEC3:
					res += 3 * sizeof(float);
					break;
				case ATRIBUTES::VEC4SI:
					res += 4 * sizeof(int);
					break;

				}
			}

			return res;
		}

		VkVertexInputBindingDescription getBinding(int bindingNumber) {

			VkVertexInputBindingDescription binding;
			binding.binding = static_cast<uint32_t>(bindingNumber);
			binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			binding.stride = getVertexStride(static_cast<uint32_t>(bindingNumber));

			return binding;
		}

		void getAttributeDescription(uint32_t bindingNumber, std::vector<VkVertexInputAttributeDescription>& atributeDescriptions) {

			uint32_t offset = 0;
			size_t i;

			if (bindingNumber > 0)
				i = atributes[bindingNumber - 1].size();

			else {
				i = 0;
			}

			for (auto& attb : atributes[bindingNumber]) {
				VkVertexInputAttributeDescription atribute{};
				atribute.binding = bindingNumber;

				switch (attb) {

				case(ATRIBUTES::VEC2):
					atribute.format = VK_FORMAT_R32G32_SFLOAT;
					atribute.location = static_cast<uint32_t>(i++);
					atribute.offset = offset;
					atributeDescriptions.push_back(atribute);
					offset += sizeof(float) * 2;
					break;

				case(ATRIBUTES::VEC3):
					atribute.format = VK_FORMAT_R32G32B32_SFLOAT;
					atribute.location = static_cast<uint32_t>(i++);
					atribute.offset = offset;
					atributeDescriptions.push_back(atribute);
					offset += sizeof(float) * 3;
					break;

				case(ATRIBUTES::VEC4):
					atribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
					atribute.location = static_cast<uint32_t>(i++);
					atribute.offset = offset;
					atributeDescriptions.push_back(atribute);
					offset += sizeof(float) * 4;
					break;
				case(ATRIBUTES::VEC4SI):
					atribute.format = VK_FORMAT_R32G32B32A32_SINT;
					atribute.location = static_cast<uint32_t>(i++);
					atribute.offset = offset;
					atributeDescriptions.push_back(atribute);
					offset += sizeof(int) * 4;
					break;
				default: break;
				}
			}

			offset = 0;
		}
	};

	struct PipelineProperties {

		const char* vertexShaderPath;
		const char* fragmentShaderPath;
		VkRenderPass* rdpass;
		std::vector<std::vector<ATRIBUTES> >atributes;
		uint32_t subpass = 0;
		VkCullModeFlagBits cullMode = VK_CULL_MODE_NONE;
		VkBool32 dephTest = VK_TRUE;
		VkFrontFace frontFaceClock = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		uint32_t colorAttachmentsCount = 1;
		uint32_t pushConstanteCount = 0;
		bool depthBias = 0;
		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
		uint32_t vertexBindingCount = 1;
		std::vector<uint32_t> vertexOffsets;
		bool alphablending = false;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	};

	enum class SHADER_TYPE {
		FRAGMENT_SHADER,
		VERTEX_SHADER,
		GEOMETRY_SHADER,
		COMPUTE_SHADER

	};

	class Shader {
		
	public:

		Shader(const Device&  _device , SHADER_TYPE _type, const std::vector<char> _code);

		VkPipelineShaderStageCreateInfo getShaderStageInfo();

		void destroyModule();
		
	private:
		const std::vector<char> code;
		const Device& device;
		SHADER_TYPE type;
		VkShaderModule vk_shaderModule;


	};

	

	class DescriptorSetLayout {
	
	public:
		DescriptorSetLayout(const VK_Objects::Device* _device, const std::vector<ShaderResource> _resources,const char *description = "GenericSet");



		std::vector< VkDescriptorSetLayoutBinding> getBindings() const;

		VkDescriptorSetLayout& getDescriptorLayoutHandle() ;

		~DescriptorSetLayout();
	
	private:

		VkDescriptorSetLayout vk_descriptorSetLayout;

		VkDescriptorType getDescriptorType(ShaderResourceType type);

		const Device* device;

		std::vector< VkDescriptorSetLayoutBinding> bindings;
	
	};

	using PDescriptorsetLayout = std::unique_ptr<VK_Objects::DescriptorSetLayout>;
	using SDescriptorsetLayout = std::shared_ptr<VK_Objects::DescriptorSetLayout>;

	class PipelineLayout {

	public:
		PipelineLayout(const Device& _device, std::vector<std::shared_ptr<DescriptorSetLayout>> _descriptors, std::vector<VkPushConstantRange> _pushConstants);
		VkPipelineLayout getHandle();

		~PipelineLayout();

	private:
		const Device& device;
		std::vector<std::shared_ptr<DescriptorSetLayout>> descriptors;
		std::vector<VkPushConstantRange> pushConstants;
		VkPipelineLayout vk_pipelineLayout;

	};

	class Pipeline {

	public:

		Pipeline(const Device& _device, std::shared_ptr<PipelineLayout>_pipelienLayout , std::unique_ptr<Shader> _vertexShader, std::unique_ptr<Shader> _fragmentShader, PipelineProperties& info, uint32_t bindingCount = 1 );
		VkPipeline& getPipelineHandle();
		std::shared_ptr<PipelineLayout> getPipelineLayoutHandle();

		const char* id;
		~Pipeline();

	private:

		std::unique_ptr<Shader> vertexShader;
		std::unique_ptr<Shader> fragmentShader;

		const Device& device;
		VkPipeline vk_pipeline;
		VkPipelineCache vk_cache;
		std::shared_ptr<PipelineLayout> pipelineLayout;
	};


	class ComputePipeline {

	public:
		ComputePipeline(const Device* _device, std::shared_ptr<PipelineLayout>_pipelienLayout, std::unique_ptr<Shader> _computeShader);


		~ComputePipeline();

	private:
		std::unique_ptr<Shader> computeShader;

		const Device* device;
		VkPipeline vk_pipeline;
		VkPipelineCache vk_cache;
		std::shared_ptr<PipelineLayout> pipelineLayout;

	};

	using PipelineManager = std::unordered_map <Pipeline, const char* >;
}
#endif