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

		CommandBuffer(VkCommandBufferLevel);

		VkCommandBuffer& getCommandBufferHandle();

	private:
		VkCommandBuffer vk_cmdBuffer;
	};

	using PComandBuffer = std::unique_ptr<VK_Objects::CommandBuffer>  ;

	class CommandPool {

	public:

		CommandPool(const Device& _device, POOL_TYPE type, VkCommandPoolCreateFlags  flags);
		
		std::unique_ptr<VK_Objects::CommandBuffer> requestCommandBuffer(VkCommandBufferLevel level)const;

		CommandPool(CommandPool&) = delete;

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