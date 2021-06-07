#pragma
#include "Graphics/VulkanFramework.h"

class RenderFrame {

public:

	RenderFrame(const VK_Objects::Device& _device,uint32_t _index);

	VkSemaphore& getRenderFinishedSemaphore();
	VkSemaphore& getImageAvaibleSemaphore();
	VkFence& getFrameCountControllFence();
	VkFence& getImageStillInFlightFence();
	uint32_t index;

	~RenderFrame();

private:
	const VK_Objects::Device& device;

	VkSemaphore vk_finishSemaphore;
	VkSemaphore vk_avaibleSemaphore;
	VkFence vk_avaibleFence;
	VkFence vk_imageStillinFlightFence;
};
using PRenderFrame = std::unique_ptr<RenderFrame>;