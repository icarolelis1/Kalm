#pragma once
#include <Graphics/VulkanFramework.h>
#include <unordered_map>
#include <Graphics/Image/Image.h>
#include <Graphics/RenderpassManager/RenderpassManager.h>
#include <Graphics/FrameBuffer/Framebuffer.h>



class FramebufferManagement

{
	using Attachments  = std::unordered_map<const char*, VK_Objects::PImage>  ;
	using Framebuffers = std::unordered_map<const char*, std::vector<VK_Objects::PFramebuffer>>;
	//using RenderPasses = std::unordered_map<const char*, std::shared_ptr<VK_Objects::Renderpass>>;




public: 
	FramebufferManagement(VK_Objects::Device* device  ,VK_Objects::SwapChain * swapChain, Game::RenderPasses  renderpasses);
	

	Attachments g_bufferImages;
	Attachments interface_images;
	Attachments depth_bufferImages;
	Attachments bloomImages;
	Attachments deferreLighting_Images;

	Framebuffers framebuffers;



private:

	VK_Objects::SwapChain* spChain;
	void createAttachemnts(VkExtent2D extent);
	void createGBufferAttachments(VkExtent2D extent);
	void createDepthMapAttachment(VkExtent2D extent);
	void createDeferredLightingAttachment(VK_Objects::SwapChain* swapChain);
	void createInterfaceAttachments(VK_Objects::SwapChain* swapChain);
	void createBloomAttachments(VK_Objects::SwapChain* swapChain);
	void createSwapChainAttachment(VK_Objects::SwapChain* swapChain);

	void createFramebuffers(VkExtent2D extent);

	const VK_Objects::Device* device;

	Attachments attachments;
	Game::RenderPasses renderpasses;
};

