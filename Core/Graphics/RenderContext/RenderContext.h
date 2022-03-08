#pragma 
#include <assert.h>
#include <functional>
#include "Graphics/RenderFrame/RenderFrame.h"
#ifndef  RENDER_CONTEXT
#define RENDER_CONTEXT


namespace RENDER {

	//Render Target holds the IMAGE , VIEW and the FRAMEBUFFERS

	class RenderContext {

	public:

		RenderContext(VK_Objects::Device & device, std::shared_ptr<VK_Objects::SwapChain> swapChain );

		RenderContext(VK_Objects::Device& device );

		std::unique_ptr<RenderContext> getContext();

		//This functions beginCommand and finishCommand can be used for local operations. Off-operations or image Transferations
		std::unique_ptr<VK_Objects::CommandBuffer> beginCommand(VK_Objects::CommandPool& pool, VkCommandBufferLevel level, VkCommandBufferUsageFlags  );
		
		void finishCommand(VK_Objects::CommandBuffer& commandBuffer, VK_Objects::CommandPool& pool);
		
		//This will perfom the render loop operation
		uint32_t draw();

		void setNumberOfFrames(int n);

		std::shared_ptr<VK_Objects::SwapChain> swapChain;

		std::vector<std::unique_ptr<RenderFrame>> frames;

		void setBuffers(std::vector<VK_Objects::SBuffer> mvpBuffers, std::vector<VK_Objects::SBuffer> lightBuffer);
		std::vector<VK_Objects::PComandBuffer> persistentCommandBuffers;

		uint32_t currentFrameIndex = 0;

	private:

		std::vector<VK_Objects::SBuffer> mvpBuffers;
		std::vector<VK_Objects::SBuffer> lightBuffers;

		const char* definiton;

		VK_Objects::Device device;


		int maxFramesInFly;

	};



}


#endif // ! RENDER_CONTEXT
