#pragma once
#include "Graphics/VulkanFramework.h"
#include "Graphics/Image/Image.h"
#include "Graphics/FrameBuffer/Framebuffer.h"

namespace RENDER {

	
	class RenderTarget {

	public:

		RenderTarget(VK_Objects::Device& device, std::vector<std::unique_ptr<VK_Objects::Image>> images, std::vector<std::unique_ptr<VK_Objects::Framebuffer>> frameBuffers, bool defaultTarget = false);

		std::unique_ptr<RenderTarget> getTarget();

		~RenderTarget();

	private:

		VK_Objects::Device& device;
		std::vector< std::unique_ptr<VK_Objects::Image>> images;
		std::vector<std::unique_ptr<VK_Objects::Framebuffer>> frameBuffers;
	};
}