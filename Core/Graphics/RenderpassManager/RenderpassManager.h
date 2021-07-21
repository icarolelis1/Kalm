#pragma once
#include <functional>
#include <unordered_map>
#include <Graphics/VulkanFramework.h>
#include <Graphics/GraphicsUtil/GraphicsUtility.h>

namespace Game {
	using RenderPasses = std::unordered_map<const char*, std::shared_ptr<VK_Objects::Renderpass>>;

	class RenderpassManager {
	public:

		RenderpassManager(const VK_Objects::Device* _device,VK_Objects::SwapChain* swapChain ,VkExtent2D extent);
		void createRenderpasses(VkExtent2D extent);

		RenderPasses passes;


	private:
		VK_Objects::SwapChain* swapChain;
		void createShadowMapRenderpass(VkExtent2D extent);
		void createGBufferRenderpass(VkExtent2D extent);
		void createDeferredLightingRenderPass(VkExtent2D extent);

		const VK_Objects::Device* device;

	};

}