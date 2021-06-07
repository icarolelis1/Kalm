#include "RenderTarget.h"

RENDER::RenderTarget::RenderTarget(VK_Objects::Device& _device, std::vector<std::unique_ptr<VK_Objects::Image>> _images, std::vector<std::unique_ptr<VK_Objects::Framebuffer>> _frameBuffers, bool defaultTarget)
	:images(std::move(_images)), frameBuffers(std::move(_frameBuffers)), device(_device)
{
	//assert(images.size() >= 1);

}

std::unique_ptr<RENDER::RenderTarget> RENDER::RenderTarget::getTarget()
{
	return std::unique_ptr<RenderTarget>(this);
}

RENDER::RenderTarget::~RenderTarget()
{
	for (auto& image : images)image->~Image();
	for (auto& fb : frameBuffers)fb->~Framebuffer();

}