#pragma 
#include "Graphics/VulkanFramework.h"
#include <assert.h>
#include <functional>
#include <Queue>
#include "Graphics/RenderFrame/RenderFrame.h"
#include <Graphics/RenderTarget/RenderTarget.h>
#include <Graphics/Image/Image.h>
#ifndef  RENDER_CONTEXT
#define RENDER_CONTEXT


namespace RENDER {

	//Render Target holds the IMAGE , VIEW and the FRAMEBUFFERS

	class RenderContext {

	public:

		RenderContext(VK_Objects::Device & device, std::shared_ptr<VK_Objects::SwapChain> swapChain );

		RenderContext(VK_Objects::Device& device );

		void setPersistentCommandBuffers(std::vector<VK_Objects::PComandBuffer> cmds);

		std::unique_ptr<RenderContext> getContext();

		//This functions beginCommand and finishCommand can be used for local operations. Off-operations or image Transferations
		std::unique_ptr<VK_Objects::CommandBuffer> beginCommand(VK_Objects::CommandPool& pool, VkCommandBufferLevel level, VkCommandBufferUsageFlags  );
		
		void finishCommand(VK_Objects::CommandBuffer& commandBuffer, VK_Objects::CommandPool& pool);
		
		//This will perfom the render loop operation
		uint32_t draw();

		void setMaxFramesInFlight(int n);

		std::shared_ptr<VK_Objects::SwapChain> swapChain;

		std::vector<PRenderFrame> frames;

		void setBuffers(std::vector<VK_Objects::SBuffer> mvpBuffers, std::vector<VK_Objects::SBuffer> lightBuffer);
		std::vector<VK_Objects::PComandBuffer> persistentCommandBuffers;

		uint32_t currentFrameIndex = 0;

	private:


		std::vector<VK_Objects::CommandPool> commandPools;
		//CommandBuffers for the main LOOP they will be reutilzed therefore their scope is the same as the RenderContext Object.

		std::vector<VK_Objects::SBuffer> mvpBuffers;
		std::vector<VK_Objects::SBuffer> lightBuffers;

		const char* definiton;

		VK_Objects::Device device;

		std::unique_ptr<RenderTarget> target;

		int maxFramesInFly;

	};

	//This object comprises the process of begining and ending a renderpass
	class SubpassRender {

	public:

		SubpassRender(const VK_Objects::Device& _device, std::unique_ptr<VK_Objects::Renderpass> _renderpass,std::unique_ptr<RenderTarget> _target);

	private:
		//This member holds explanations of what is this Renderpass about
		const char* definition;

	};


}


#endif // ! RENDER_CONTEXT
