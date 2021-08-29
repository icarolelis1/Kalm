#include "RenderpassManager.h"

Game::RenderpassManager::RenderpassManager(const VK_Objects::Device* _device, VK_Objects::SwapChain* _swapChain, VkExtent2D extent):device(_device)
{
	//Creates all scene renderpasses
	//Extent refers to the final output extent.
	swapChain = _swapChain;
	createRenderpasses(extent);
}

void Game::RenderpassManager::createRenderpasses(VkExtent2D extent )
{
	createShadowMapRenderpass(extent);
	createGBufferRenderpass(extent);
	createDeferredLightingRenderPass(extent);
	createImGuiRenderpass(extent);
	createBloomRenderpasses(extent);
	createSwapChainRenderPass(extent);

}

void Game::RenderpassManager::createShadowMapRenderpass(VkExtent2D extent)
{
	//Creates a renderpass for variance shadow map. Two float values will be stored.

	std::unique_ptr<VK_Objects::Renderpass> shadowMapRenderpass = std::make_unique<VK_Objects::Renderpass>(device, "SHADOW_MAP", extent);

	VK_Objects::RenderpassProperties renderpassProperties;

	//Single Attachment
	renderpassProperties.attachments.resize(2);

	VkFormat format = device->getSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);


	VK_Objects::RenderAttachment depthAttachment;
	depthAttachment.description.format = format;
	depthAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	depthAttachment.description.flags = 0;

	depthAttachment.reference.attachment = 1;
	depthAttachment.reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VK_Objects::RenderAttachment squaredDepthAttachment;
	squaredDepthAttachment.description.format =VK_FORMAT_R16_SFLOAT;
	squaredDepthAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	squaredDepthAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	squaredDepthAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	squaredDepthAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	squaredDepthAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	squaredDepthAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	squaredDepthAttachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	squaredDepthAttachment.description.flags = 0;

	squaredDepthAttachment.reference.attachment = 0;
	squaredDepthAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;



	VK_Objects::Subpass subpass;
	subpass.description.resize(1);
	subpass.dependencies.resize(1);

	VkAttachmentReference depthReferemces[1] = { depthAttachment.reference};

	subpass.description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.description[0].pDepthStencilAttachment = &depthReferemces[0];
	subpass.description[0].colorAttachmentCount = 1;
	subpass.description[0].pColorAttachments = &squaredDepthAttachment.reference;

	subpass.dependencies[0].srcSubpass =		VK_SUBPASS_EXTERNAL;
	subpass.dependencies[0].dstSubpass =		0;
	subpass.dependencies[0].srcStageMask =		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	subpass.dependencies[0].dstStageMask =		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpass.dependencies[0].srcAccessMask =		VK_ACCESS_MEMORY_WRITE_BIT;
	subpass.dependencies[0].dstAccessMask =		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ;
	subpass.dependencies[0].dependencyFlags =	VK_DEPENDENCY_BY_REGION_BIT;



	renderpassProperties.attachments[1] = depthAttachment;
	renderpassProperties.attachments[0] = squaredDepthAttachment;
	shadowMapRenderpass->properties = renderpassProperties;
	shadowMapRenderpass->subpass = subpass;

	Vk_Functions::createRenderpass(device, *shadowMapRenderpass.get());

	passes[shadowMapRenderpass->getKey()] = std::move(shadowMapRenderpass);
}

void Game::RenderpassManager::createGBufferRenderpass(VkExtent2D extent)
{

	//Creates renderpass to be used in GBUFFER creation.
	//Gbuffer will store : 
	//8 bit precision NORMALS;
	//8Bit Precision vec2 with Albedo, and metallicness
	
	//Creates a renderpass for variance shadow map. Two float values will be stored.

	std::unique_ptr<VK_Objects::Renderpass> gbufferRenderpass = std::make_unique<VK_Objects::Renderpass>(device, "G_BUFFER", extent);

	VK_Objects::RenderpassProperties renderpassProperties;

	//Single Attachment
	renderpassProperties.attachments.resize(4);

	VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;

	VK_Objects::RenderAttachment depthAttachment;
	depthAttachment.description.format = device->getSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);


	depthAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	depthAttachment.description.flags = 0;

	depthAttachment.reference.attachment = 3;
	depthAttachment.reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VK_Objects::RenderAttachment metallicRoughnessAttachment;
	metallicRoughnessAttachment.description.format = VK_FORMAT_R8G8_UNORM;
	metallicRoughnessAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	metallicRoughnessAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	metallicRoughnessAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	metallicRoughnessAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	metallicRoughnessAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	metallicRoughnessAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	metallicRoughnessAttachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	metallicRoughnessAttachment.description.flags = 0;

	metallicRoughnessAttachment.reference.attachment = 1;
	metallicRoughnessAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VK_Objects::RenderAttachment NormalsAttachment;
	NormalsAttachment.description.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	NormalsAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	NormalsAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	NormalsAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	NormalsAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	NormalsAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	NormalsAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	NormalsAttachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	NormalsAttachment.description.flags = 0;

	NormalsAttachment.reference.attachment = 2;
	NormalsAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	VK_Objects::RenderAttachment albedoAttachment;
	albedoAttachment.description.format = VK_FORMAT_R8G8B8A8_UNORM;
	albedoAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	albedoAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	albedoAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	albedoAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	albedoAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	albedoAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	albedoAttachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	albedoAttachment.description.flags = 0;

	albedoAttachment.reference.attachment = 0;
	albedoAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VK_Objects::Subpass subpass;
	subpass.description.resize(1);
	subpass.dependencies.resize(2);

	VkAttachmentReference pColorAttachments[3] = {  albedoAttachment.reference , metallicRoughnessAttachment.reference , NormalsAttachment.reference };


	subpass.description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.description[0].colorAttachmentCount = 3;
	subpass.description[0].pColorAttachments = pColorAttachments; 
	subpass.description[0].pDepthStencilAttachment = &depthAttachment.reference;


		
	subpass.dependencies[0].srcSubpass =		VK_SUBPASS_EXTERNAL;
	subpass.dependencies[0].dstSubpass =		0;
	subpass.dependencies[0].srcStageMask =		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	subpass.dependencies[0].dstStageMask =		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass.dependencies[0].srcAccessMask =		VK_ACCESS_MEMORY_READ_BIT;
	subpass.dependencies[0].dstAccessMask =		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;
	subpass.dependencies[0].dependencyFlags=	VK_DEPENDENCY_BY_REGION_BIT;

	subpass.dependencies[1].srcSubpass = 0;
	subpass.dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpass.dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass.dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpass.dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;
	subpass.dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpass.dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	renderpassProperties.attachments[0] = albedoAttachment;
	renderpassProperties.attachments[1] = metallicRoughnessAttachment;
	renderpassProperties.attachments[2] = NormalsAttachment;
	renderpassProperties.attachments[3] = depthAttachment;

	gbufferRenderpass->properties = renderpassProperties;
	gbufferRenderpass->subpass = subpass;

	Vk_Functions::createRenderpass(device, *gbufferRenderpass.get());

	passes[gbufferRenderpass->getKey()] = std::move(gbufferRenderpass);


}

void Game::RenderpassManager::createDeferredLightingRenderPass(VkExtent2D extent)
{

	//Creates renderpass to be used in DeferredLighting creation.

	std::unique_ptr<VK_Objects::Renderpass> deferredLightingRenderPass = std::make_unique<VK_Objects::Renderpass>(device, "DEFERRED_LIGHTING", extent);

	VK_Objects::RenderpassProperties renderpassProperties;

	renderpassProperties.attachments.resize(2);

	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	
	VK_Objects::RenderAttachment deferredLightingAttachment;
	deferredLightingAttachment.description.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	deferredLightingAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	deferredLightingAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	deferredLightingAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	deferredLightingAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	deferredLightingAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	deferredLightingAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	deferredLightingAttachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	deferredLightingAttachment.description.flags = 0;

	deferredLightingAttachment.reference.attachment = 0;
	deferredLightingAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VK_Objects::RenderAttachment brightnessAttachment;
	brightnessAttachment.description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	brightnessAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	brightnessAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	brightnessAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	brightnessAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	brightnessAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	brightnessAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	brightnessAttachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	brightnessAttachment.description.flags = 0;

	brightnessAttachment.reference.attachment = 1;
	brightnessAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VK_Objects::Subpass subpass;
	subpass.description.resize(1);
	subpass.dependencies.resize(2);

	VkAttachmentReference pColorAttachments[2] = { deferredLightingAttachment.reference,brightnessAttachment.reference };


	subpass.description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.description[0].colorAttachmentCount =2;
	subpass.description[0].pColorAttachments = pColorAttachments;



	subpass.dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass.dependencies[0].dstSubpass = 0;
	subpass.dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpass.dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass.dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpass.dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;
	subpass.dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


	subpass.dependencies[1].srcSubpass = 0;
	subpass.dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpass.dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass.dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpass.dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;
	subpass.dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpass.dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	renderpassProperties.attachments[0] = deferredLightingAttachment;
	renderpassProperties.attachments[1] = brightnessAttachment;


	deferredLightingRenderPass->properties = renderpassProperties;
	deferredLightingRenderPass->subpass = subpass;


	Vk_Functions::createRenderpass(device, *deferredLightingRenderPass.get());

	passes[deferredLightingRenderPass->getKey()] = std::move(deferredLightingRenderPass);

}

void Game::RenderpassManager::createImGuiRenderpass(VkExtent2D extent)
{


	//Creates renderpass to be used in DeferredLighting creation.

	std::unique_ptr<VK_Objects::Renderpass> imGuiRenderpass = std::make_unique<VK_Objects::Renderpass>(device, "INTERFACE", extent);

	VK_Objects::RenderpassProperties renderpassProperties;

	//Single Attachment
	renderpassProperties.attachments.resize(1);

	VkFormat format = swapChain->getFormat();

	VK_Objects::RenderAttachment interfaceAttachment;
	interfaceAttachment.description.format = format;
	interfaceAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	interfaceAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	interfaceAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	interfaceAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	interfaceAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	interfaceAttachment.description.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	interfaceAttachment.description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	interfaceAttachment.description.flags = 0;

	interfaceAttachment.reference.attachment = 0;
	interfaceAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VK_Objects::Subpass subpass;
	subpass.description.resize(1);
	subpass.dependencies.resize(1);

	VkAttachmentReference pColorAttachments[1] = { interfaceAttachment.reference };

	subpass.description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.description[0].colorAttachmentCount = 1;
	subpass.description[0].pColorAttachments = pColorAttachments;

	subpass.dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass.dependencies[0].dstSubpass = 0;
	subpass.dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass.dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass.dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass.dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderpassProperties.attachments[0] = interfaceAttachment;

	imGuiRenderpass->properties = renderpassProperties;
	imGuiRenderpass->subpass = subpass;

	Vk_Functions::createRenderpass(device, *imGuiRenderpass.get());

	passes[imGuiRenderpass->getKey()] = std::move(imGuiRenderpass);

}

void Game::RenderpassManager::createBloomRenderpasses(VkExtent2D _extent)
{

	VkExtent2D extent = _extent;
	extent.width /= 2;
	extent.height /= 2;

	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

	std::unique_ptr<VK_Objects::Renderpass> verticalBlurRenderpass = std::make_unique<VK_Objects::Renderpass>(device, "VERTICAL_BLUR", extent);

	VK_Objects::RenderpassProperties renderpassProperties;

	VK_Objects::RenderAttachment verticalBlurPass;

	verticalBlurPass.description.format = format;
	verticalBlurPass.description.samples = VK_SAMPLE_COUNT_1_BIT;
	verticalBlurPass.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	verticalBlurPass.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	verticalBlurPass.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	verticalBlurPass.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	verticalBlurPass.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	verticalBlurPass.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	verticalBlurPass.description.flags = 0;

	verticalBlurPass.reference.attachment = 0;
	verticalBlurPass.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VK_Objects::Subpass subpass;
	subpass.description.resize(1);
	subpass.dependencies.resize(2);

	VkAttachmentReference pColorAttachments[1] = { verticalBlurPass.reference };

	subpass.description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.description[0].colorAttachmentCount = 1;
	subpass.description[0].pColorAttachments = pColorAttachments;

	subpass.dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass.dependencies[0].dstSubpass = 0;
	subpass.dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpass.dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass.dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpass.dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass.dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	
	subpass.dependencies[1].srcSubpass = 0;
	subpass.dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpass.dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass.dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpass.dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass.dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpass.dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	renderpassProperties.attachments.resize(1);
	renderpassProperties.attachments[0] = verticalBlurPass;

	verticalBlurRenderpass->properties = renderpassProperties;
	verticalBlurRenderpass->subpass = subpass;

	Vk_Functions::createRenderpass(device, *verticalBlurRenderpass.get());

	passes[verticalBlurRenderpass->getKey()] = std::move(verticalBlurRenderpass);




	{
		//Horizontal Blur

		std::unique_ptr<VK_Objects::Renderpass> horizontalBlurRenderpass = std::make_unique<VK_Objects::Renderpass>(device, "HORIZONTAL_BLUR",extent);

		VK_Objects::RenderpassProperties renderpassProperties;
		renderpassProperties.attachments.resize(1);
		VK_Objects::RenderAttachment horizontalBlurPas;

		horizontalBlurPas.description.format = format;
		horizontalBlurPas.description.samples = VK_SAMPLE_COUNT_1_BIT;
		horizontalBlurPas.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		horizontalBlurPas.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		horizontalBlurPas.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		horizontalBlurPas.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		horizontalBlurPas.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		horizontalBlurPas.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		horizontalBlurPas.description.flags = 0;

		horizontalBlurPas.reference.attachment = 0;
		horizontalBlurPas.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VK_Objects::Subpass subpass;
		subpass.description.resize(1);
		subpass.dependencies.resize(2);

		VkAttachmentReference pColorAttachments[1] = { horizontalBlurPas.reference };

		subpass.description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.description[0].colorAttachmentCount = 1;
		subpass.description[0].pColorAttachments = pColorAttachments;

		subpass.dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		subpass.dependencies[0].dstSubpass = 0;
		subpass.dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpass.dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass.dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpass.dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpass.dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		subpass.dependencies[1].srcSubpass = 0;
		subpass.dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		subpass.dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass.dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpass.dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpass.dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpass.dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		renderpassProperties.attachments[0] = horizontalBlurPas;

		horizontalBlurRenderpass->properties = renderpassProperties;
		horizontalBlurRenderpass->subpass = subpass;

		Vk_Functions::createRenderpass(device, *horizontalBlurRenderpass.get());

		passes[horizontalBlurRenderpass->getKey()] = std::move(horizontalBlurRenderpass);


	}

}

void Game::RenderpassManager::createSwapChainRenderPass(VkExtent2D extent)
{



	std::unique_ptr<VK_Objects::Renderpass> swapChainRenderpass = std::make_unique<VK_Objects::Renderpass>(device, "SWAPCHAIN_RENDERPASS", extent);

	VK_Objects::RenderpassProperties renderpassProperties;


	renderpassProperties.attachments.resize(1);

	VkFormat format = swapChain->getFormat();

	VK_Objects::RenderAttachment renderpassAttachment;
	renderpassAttachment.description.format = format;
	renderpassAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	renderpassAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	renderpassAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	renderpassAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	renderpassAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	renderpassAttachment.description.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	renderpassAttachment.description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	renderpassAttachment.description.flags = 0;

	renderpassAttachment.reference.attachment = 0;
	renderpassAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VK_Objects::Subpass subpass;
	subpass.description.resize(1);
	subpass.dependencies.resize(1);

	VkAttachmentReference pColorAttachments[1] = { renderpassAttachment.reference };

	subpass.description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.description[0].colorAttachmentCount = 1;
	subpass.description[0].pColorAttachments = pColorAttachments;

	subpass.dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass.dependencies[0].dstSubpass = 0;
	subpass.dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass.dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass.dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass.dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderpassProperties.attachments[0] = renderpassAttachment;

	swapChainRenderpass->properties = renderpassProperties;
	swapChainRenderpass->subpass = subpass;

	Vk_Functions::createRenderpass(device, *swapChainRenderpass.get());

	passes[swapChainRenderpass->getKey()] = std::move(swapChainRenderpass);

}
