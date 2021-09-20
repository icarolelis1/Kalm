#pragma once
#include <functional>
#include <unordered_map>
#include <Graphics/VulkanFramework.h>
#include <Graphics/GraphicsUtil/GraphicsUtility.h>

namespace Game {
	using RenderPasses = std::unordered_map<const char*, std::shared_ptr<VK_Objects::Renderpass>>;

	class RenderpassManager {
	public:

		RenderpassManager(const VK_Objects::Device* _device,VK_Objects::SwapChain* swapChain ,VkExtent2D extent, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
		void createRenderpasses(VkExtent2D extent);

		RenderPasses passes;


	private:
		VkSampleCountFlagBits sampleCount;
		VK_Objects::SwapChain* swapChain;
		void createShadowMapRenderpass(VkExtent2D extent);
		void createGBufferRenderpass(VkExtent2D extent);
		void createDeferredLightingRenderPass(VkExtent2D extent);
		void createImGuiRenderpass(VkExtent2D extent);
		void createBloomRenderpasses(VkExtent2D extend);
		void createSwapChainRenderPass(VkExtent2D extent);

		const VK_Objects::Device* device;

	};

}