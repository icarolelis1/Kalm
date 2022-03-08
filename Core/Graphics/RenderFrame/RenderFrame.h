#pragma
#include "Graphics/CommandPool/CommandPool.h"
class RenderFrame {
	
public:

	RenderFrame(const VK_Objects::Device& _device,uint32_t _index);

	VkSemaphore& getRenderFinishedSemaphore();
	VkSemaphore& getImageAvaibleSemaphore();
	VkFence& getFrameCountControllFence();
	VkFence& getImageStillInFlightFence();

	VK_Objects::CommandBuffer& getCommandBuffer();
	VkCommandBuffer&	 getCommandBufferHandler();

	uint32_t index;

	~RenderFrame();

private:
	std::vector<VK_Objects::CommandBuffer> commands;
	const VK_Objects::Device& device;
	std::vector<std::shared_ptr<VK_Objects::CommandPool>> commandPools;
	VkSemaphore vk_finishSemaphore;
	VkSemaphore vk_avaibleSemaphore;
	VkFence vk_avaibleFence;
	VkFence vk_imageStillinFlightFence;
};
using PRenderFrame = std::unique_ptr<RenderFrame>;