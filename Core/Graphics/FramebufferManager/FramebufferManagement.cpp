#include "FramebufferManagement.h"

FramebufferManagement::FramebufferManagement( VK_Objects::Device * _device , VK_Objects::SwapChain* swapChain, Game::RenderPasses _renderpasses):device(_device),spChain(swapChain),renderpasses(_renderpasses)
{
	createAttachemnts(swapChain->getExtent());
	createSwapChainAttachment(swapChain);

}

void FramebufferManagement::createAttachemnts(VkExtent2D extent)
{
	createGBufferAttachments(extent);
	createDepthMapAttachment(extent);
}


void FramebufferManagement::createGBufferAttachments(VkExtent2D extent)
{
	//Image(const Device* device, uint32_t Width, uint32_t Height, ImageFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageCreateFlags flags, VkImageAspectFlags aspectFlags, uint32_t arrayLayers = 1, bool useMaxNumMips = 0);
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

	VK_Objects::PImage normals = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |VK_IMAGE_USAGE_SAMPLED_BIT , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1,0 );

	VK_Objects::PImage albedo = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);
	
	VK_Objects::PImage metallicRoughness = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, VK_FORMAT_R8G8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);

	VkFormat depthFormat = device->getSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	VK_Objects::PImage depthImage = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 0);


	//Framebuffer(VK_Objects::Device * _device, VkImageView & view, VK_Objects::Renderpass * renderpass, VkExtent2D _extent);

	//Creates one Framebuffer per image on SwapChain
	int n = spChain->getNumberOfImages();

	for (int i = 0; i < n; i++) {
	

		VkImageView attachments[4] = { *albedo->getVkImageViewHandle(), *metallicRoughness->getVkImageViewHandle(), *normals->getVkImageViewHandle(),*depthImage->getVkImageViewHandle() };
	
		framebuffers["G_BUFFER"].push_back(std::move(std::make_unique<VK_Objects::Framebuffer>(device,4,attachments,renderpasses["G_BUFFER"],extent)));
	
	}

	//Move the created images to g_bufferImages unordered map.
	g_bufferImages["NORMALS"] = std::move(normals);
	g_bufferImages["ALBEDO"] = std::move(albedo);
	g_bufferImages["METALLICROUGHNESS"] = std::move(metallicRoughness);
	g_bufferImages["DEPTH"] = std::move(depthImage);


}

void FramebufferManagement::createDepthMapAttachment(VkExtent2D extent)
{
	VkFormat format = device->getSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	VK_Objects::PImage depth = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 0);
	VK_Objects::PImage depthSquared = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, VK_FORMAT_R16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT| VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);

	int n = spChain->getNumberOfImages();
	for (int i = 0; i < n; i++) {

		VkImageView attachments[2] = {*depthSquared->getVkImageViewHandle(),*depth->getVkImageViewHandle() };

		VK_Objects::Framebuffer f(device, 2,attachments, renderpasses["SHADOW_MAP"], extent);
		framebuffers["SHADOW_MAP"].push_back(std::move(std::make_unique<VK_Objects::Framebuffer>(device,2,attachments,renderpasses["SHADOW_MAP"],extent)));

	}
	depth_bufferImages["DEPTH"] = std::move(depth);
	depth_bufferImages["DEPTH_SQUARED"] = std::move(depthSquared);

}

void FramebufferManagement::createSwapChainAttachment(VK_Objects::SwapChain* swapChain)
{

	//Creates one Framebuffer per image on SwapChain
	int n = spChain->getNumberOfImages();
	for (int i = 0; i < n; i++) {


		VkImageView attachments[1] = { swapChain->getViews()[i] };

		framebuffers["SWAPCHAIN_FRAMEBUFFER"].push_back(std::move(std::make_unique<VK_Objects::Framebuffer>(device, 1, attachments, renderpasses["DEFERRED_LIGHTING"], swapChain->getExtent())));

	}



}

void FramebufferManagement::createFramebuffers(VkExtent2D extent)
{

	int n = spChain->getNumberOfImages();

}

