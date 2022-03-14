#include "FramebufferManagement.h"

FramebufferManagement::FramebufferManagement( VK_Objects::Device * _device , VK_Objects::SwapChain* swapChain, Game::RenderPasses _renderpasses,VkSampleCountFlagBits _samples):device(_device),spChain(swapChain),renderpasses(_renderpasses),maxSampleCount(_samples)
{
	std::cout << swapChain->getExtent().width << std::endl;
	createAttachemnts(swapChain->getExtent());
	createDeferredLightingAttachment(swapChain);
	createInterfaceAttachments(swapChain);
	createBloomAttachments(swapChain);
	createSwapChainAttachment(swapChain);
	createSSDOAttachments(swapChain);
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

	VK_Objects::PImage normals = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |VK_IMAGE_USAGE_SAMPLED_BIT , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1,0 );

	VK_Objects::PImage albedo = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);
	
	VK_Objects::PImage metallicRoughness = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);
	
	VK_Objects::PImage emissionAttachment = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);

	VkFormat depthFormat = device->getSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	VK_Objects::PImage depthImage = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 0);

	int n = spChain->getNumberOfImages();

	for (int i = 0; i < n; i++) {
	

		VkImageView attachments[5] = { *albedo->getVkImageViewHandle(), *metallicRoughness->getVkImageViewHandle(), *normals->getVkImageViewHandle(),*emissionAttachment->getVkImageViewHandle(),  *depthImage->getVkImageViewHandle() };
	
		framebuffers["G_BUFFER"].push_back(std::move(std::make_unique<VK_Objects::Framebuffer>(device,5,attachments,renderpasses["G_BUFFER"],extent)));
	
	}

	//Move the created images to g_bufferImages unordered map.
	g_bufferImages["NORMALS"] = std::move(normals);
	g_bufferImages["ALBEDO"] = std::move(albedo);
	g_bufferImages["METALLICROUGHNESS"] = std::move(metallicRoughness);
	g_bufferImages["EMISSION"] = std::move(emissionAttachment);
	g_bufferImages["DEPTH"] = std::move(depthImage);


}

void FramebufferManagement::createDepthMapAttachment(VkExtent2D _extent)
{
	VkExtent2D extent;
	extent.width = 2048;
	extent.height = 2048;

	VkFormat format = device->getSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	VK_Objects::PImage depth = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 0);
	VK_Objects::PImage depthSquared = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT| VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);

	int n = spChain->getNumberOfImages();
	for (int i = 0; i < n; i++) {

		VkImageView attachments[2] = {*depthSquared->getVkImageViewHandle(),*depth->getVkImageViewHandle() };

		VK_Objects::Framebuffer f(device, 2,attachments, renderpasses["SHADOW_MAP"], extent);
		framebuffers["SHADOW_MAP"].push_back(std::move(std::make_unique<VK_Objects::Framebuffer>(device,2,attachments,renderpasses["SHADOW_MAP"],extent)));

	}

	depth_bufferImages["DEPTH"] = std::move(depth);
	depth_bufferImages["DEPTH_SQUARED"] = std::move(depthSquared);

}

void FramebufferManagement::createDeferredLightingAttachment(VK_Objects::SwapChain* swapChain)
{

	//Creates one Framebuffer per image on SwapChain
	int n = spChain->getNumberOfImages();
	VkExtent2D extent = swapChain->getExtent();

	VK_Objects::PImage deferredLightingImage = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);
	VK_Objects::PImage brightnessImage = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);

	for (int i = 0; i < n; i++) {


		VkImageView attachments[2] = { *deferredLightingImage->getVkImageViewHandle(),*brightnessImage->getVkImageViewHandle() };

		framebuffers["DEFERRED_LIGHTING_FRAMEBUFFER"].push_back(std::move(std::make_unique<VK_Objects::Framebuffer>(device, 2, attachments, renderpasses["DEFERRED_LIGHTING"], swapChain->getExtent())));

	}

	deferreLighting_Images["DEFERRED_LIGHTING_ATTACHMENT"] = std::move(deferredLightingImage);
	deferreLighting_Images["DEFERRED_BRIGHTNESS_ATTACHMENT"] = std::move(brightnessImage);


	/*

	VkImageView attachments[1] = { swapChain->getViews()[i] };

	framebuffers["DEFERRED_LIGHTING_FRAMEBUFFER"].push_back(std::move(std::make_unique<VK_Objects::Framebuffer>(device, 1, attachments, renderpasses["DEFERRED_LIGHTING"], swapChain->getExtent())));
	*/

}

void FramebufferManagement::createInterfaceAttachments(VK_Objects::SwapChain* swapChain)
{

	VkExtent2D extent = swapChain->getExtent();
	VkFormat format = swapChain->getFormat();

	//VK_Objects::PImage interface_UI = std::make_unique<VK_Objects::Image>(device, extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);

	int n = spChain->getNumberOfImages();

	for (int i = 0; i < n; i++) {


		VkImageView attachments[1] = { swapChain->getViews()[i] };

		framebuffers["INTERFACE"].push_back(std::move(std::make_unique<VK_Objects::Framebuffer>(device, 1, attachments, renderpasses["INTERFACE"], extent)));

	}

	//Move the created images to g_bufferImages unordered map.



}

void FramebufferManagement::createBloomAttachments(VK_Objects::SwapChain* swapChain)
{
	//Image(const Device* device, uint32_t Width, uint32_t Height, ImageFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageCreateFlags flags, VkImageAspectFlags aspectFlags, uint32_t arrayLayers = 1, bool useMaxNumMips = 0);
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	VkExtent2D partion_extent = swapChain->getExtent();
	partion_extent.width /= 2;
	partion_extent.height /= 2;


	VK_Objects::PImage verticalImage = std::make_unique<VK_Objects::Image>(device, partion_extent.width, partion_extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);
	VK_Objects::PImage horizontalImage = std::make_unique<VK_Objects::Image>(device, partion_extent.width, partion_extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);

	int n = spChain->getNumberOfImages();

	for (int i = 0; i < n; i++) {


		VkImageView attachments[1] = { *verticalImage->getVkImageViewHandle()};
		VkImageView attachments_[1] = { *horizontalImage->getVkImageViewHandle() };

		framebuffers["VERTICAL_FRAMEBUFFER"].push_back(std::move(std::make_unique<VK_Objects::Framebuffer>(device, 1, attachments, renderpasses["VERTICAL_BLUR"], partion_extent)));
		framebuffers["HORIZONTAL_FRAMEBUFFER"].push_back(std::move(std::make_unique<VK_Objects::Framebuffer>(device, 1, attachments_, renderpasses["HORIZONTAL_BLUR"], partion_extent)));

	}

	//Move the created images to g_bufferImages unordered map.
	bloomImages["VERTICAL_BLOOM"] = std::move(verticalImage);
	bloomImages["HORIZONTAL_BLOOM"] = std::move(horizontalImage);

}

void FramebufferManagement::createSwapChainAttachment(VK_Objects::SwapChain* swapChain)
{

	VkExtent2D extent = swapChain->getExtent();
	VkFormat format = swapChain->getFormat();


	int n = spChain->getNumberOfImages();

	for (int i = 0; i < n; i++) {


		VkImageView attachments[1] = { swapChain->getViews()[i] };

		framebuffers["SWAPCHAIN_FRAMEBUFFER"].push_back(std::move(std::make_unique<VK_Objects::Framebuffer>(device, 1, attachments, renderpasses["SWAPCHAIN_RENDERPASS"], extent)));

	}

	//Move the created images to g_bufferImages unordered map.
	//msaa_Images["MSAA_IMAGES"] = std::move(msaaImage);


}

void FramebufferManagement::createSSDOAttachments(VK_Objects::SwapChain* swapChain)
{

	//Image(const Device* device, uint32_t Width, uint32_t Height, ImageFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageCreateFlags flags, VkImageAspectFlags aspectFlags, uint32_t arrayLayers = 1, bool useMaxNumMips = 0);
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	VkExtent2D partion_extent = swapChain->getExtent();



	VK_Objects::PImage ssdoImage = std::make_unique<VK_Objects::Image>(device, partion_extent.width, partion_extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);

	int n = spChain->getNumberOfImages();

	for (int i = 0; i < n; i++) {


		VkImageView attachments[1] = { *ssdoImage->getVkImageViewHandle() };

		framebuffers["SSDO"].push_back(std::move(std::make_unique<VK_Objects::Framebuffer>(device, 1, attachments, renderpasses["SSDO"], partion_extent)));

	}

	//Move the created images to g_bufferImages unordered map.
	ssdo_images["VERTICAL_BLOOM"] = std::move(ssdoImage);
}

void FramebufferManagement::createFramebuffers(VkExtent2D extent)
{

	int n = spChain->getNumberOfImages();

}

