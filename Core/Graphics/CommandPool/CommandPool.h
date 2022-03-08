#include "Graphics/VulkanFramework.h"
#ifndef COMMAND_POOL
#define COMMAND_POOL

namespace VK_Objects {


	enum class POOL_TYPE
	{
		GRAPHICS,
		COMPUTE,
		TRANSFER
	};

	enum class COMMAND_STATE
	{
		INITIAL,
		RECORDING,
		PENDING
	};

	class CommandBuffer {

	public:

		CommandBuffer();



		VkCommandBuffer& getCommandBufferHandle();

	private:
		VkCommandBuffer vk_cmdBuffer;
	};

	using PComandBuffer = std::unique_ptr<VK_Objects::CommandBuffer>  ;

	class CommandPool {

	public:

		CommandPool(const Device& _device, POOL_TYPE type, VkCommandPoolCreateFlags  flags);
		CommandPool(const CommandPool&) =delete;
		CommandPool( CommandPool&&)=delete;

		std::unique_ptr<VK_Objects::CommandBuffer> requestCommandBuffer(VkCommandBufferLevel level)const;
		VkCommandBuffer requestCommandBufferVK(VkCommandBufferLevel level) const;


		VkCommandPool& getPoolHanndle();

		void allocateCommandBuffer(std::vector<CommandBuffer>& cmdBuffers, VkCommandBufferLevel    level) const;
		void allocateCommandBuffer(VkCommandBuffer& cmdBuffer, VkCommandBufferLevel    level) const;


		~CommandPool();

	private:

	const VK_Objects::Device& device;
	VkCommandPool vk_cmdPool;

	};

	using PCommandPool = std::unique_ptr<VK_Objects::CommandPool>;

}
#endif