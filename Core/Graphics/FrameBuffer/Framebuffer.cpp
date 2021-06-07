#include "Framebuffer.h"

VK_Objects::Framebuffer::Framebuffer(const VK_Objects::Device* _device, VK_Objects::Image* _image, const VK_Objects::Renderpass* _renderpass):device(_device)
{
	VkExtent2D e = _image->getExtent();

	VkFramebufferCreateInfo fbufCreateInfo{};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.renderPass = _renderpass->vk_renderpass;
	fbufCreateInfo.attachmentCount = 1;
	fbufCreateInfo.pAttachments = _image->getVkImageViewHandle();
	fbufCreateInfo.width = e.width;;
	fbufCreateInfo.height = e.height;
	fbufCreateInfo.layers = _image->getNumberOfLayers();

	VkResult r = vkCreateFramebuffer(device->getLogicalDevice(), &fbufCreateInfo, device->getAllocator(), &vk_Framebuffer);

	if (r != VK_SUCCESS) {
		std::cout << "    Failed to create FrameBuffer\n";
	}

}

VK_Objects::Framebuffer::Framebuffer(const VK_Objects::Device* _device, uint32_t attachmentCoount ,const VkImageView* view, std::shared_ptr<VK_Objects::Renderpass> renderpass, VkExtent2D e):device(_device)
{
	VkFramebufferCreateInfo fbufCreateInfo{};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.renderPass = renderpass->vk_renderpass;
	fbufCreateInfo.attachmentCount = attachmentCoount;
	fbufCreateInfo.pAttachments = view;
	fbufCreateInfo.width = e.width;;
	fbufCreateInfo.height = e.height;
	fbufCreateInfo.layers = 1;

	VkResult r = vkCreateFramebuffer(device->getLogicalDevice(), &fbufCreateInfo, device->getAllocator(), &vk_Framebuffer);

	if (r != VK_SUCCESS) {
		std::cout << "    Failed to create FrameBuffer\n";
	}
}

VkFramebuffer& VK_Objects::Framebuffer::getFramebufferHandle()
{
	return vk_Framebuffer;
}

VK_Objects::Framebuffer::~Framebuffer()
{
	vkDestroyFramebuffer(device->getLogicalDevice(), vk_Framebuffer, device->getAllocator());
}
