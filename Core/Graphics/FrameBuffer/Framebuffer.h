#pragma once
#include "Graphics/Image/Image.h"

namespace VK_Objects {

	class Framebuffer
	{
	public:
		Framebuffer(const VK_Objects::Device* _device, VK_Objects::Image* _image, const VK_Objects::Renderpass* _renderpass);
		
		Framebuffer(const VK_Objects::Device* _device, uint32_t attachmentCoount, const VkImageView* view,  std::shared_ptr<VK_Objects::Renderpass> renderpass, VkExtent2D _extent);

		VkFramebuffer& getFramebufferHandle();

		std::string name;
		~Framebuffer();

	private:
		VkFramebuffer vk_Framebuffer;
		const VK_Objects::Device* device;

	};
	using PFramebuffer = std::unique_ptr<Framebuffer>;

}
