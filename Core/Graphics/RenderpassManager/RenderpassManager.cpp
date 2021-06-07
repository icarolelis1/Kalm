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
	depthAttachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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
	renderpassProperties.attachments.resize(5);

	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

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

	depthAttachment.reference.attachment = 4;
	depthAttachment.reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VK_Objects::RenderAttachment metallicRoughnessAttachment;
	metallicRoughnessAttachment.description.format = VK_FORMAT_R8G8_UNORM;
	metallicRoughnessAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	metallicRoughnessAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	metallicRoughnessAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	metallicRoughnessAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	metallicRoughnessAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	metallicRoughnessAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	metallicRoughnessAttachment.description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	metallicRoughnessAttachment.description.flags = 0;

	metallicRoughnessAttachment.reference.attachment =2;
	metallicRoughnessAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VK_Objects::RenderAttachment NormalsAttachment;
	NormalsAttachment.description.format = format;
	NormalsAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	NormalsAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	NormalsAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	NormalsAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	NormalsAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	NormalsAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	NormalsAttachment.description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	NormalsAttachment.description.flags = 0;

	NormalsAttachment.reference.attachment = 3;
	NormalsAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	VK_Objects::RenderAttachment albedoAttachment;
	albedoAttachment.description.format = format;
	albedoAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	albedoAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	albedoAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	albedoAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	albedoAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	albedoAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	albedoAttachment.description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	albedoAttachment.description.flags = 0;

	albedoAttachment.reference.attachment = 1;
	albedoAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//Final output of the renderpass
	VK_Objects::RenderAttachment colorAttachemnt;
	colorAttachemnt.description.format = swapChain->getFormat();
	colorAttachemnt.description.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachemnt.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachemnt.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachemnt.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachemnt.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachemnt.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachemnt.description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	colorAttachemnt.description.flags = 0;

	colorAttachemnt.reference.attachment = 0;
	colorAttachemnt.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VK_Objects::Subpass subpass;
	subpass.description.resize(2);
	subpass.dependencies.resize(3);

	VkAttachmentReference pColorAttachments[4] = { colorAttachemnt.reference, albedoAttachment.reference , metallicRoughnessAttachment.reference , NormalsAttachment.reference };


	subpass.description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.description[0].colorAttachmentCount =4;
	subpass.description[0].pColorAttachments = pColorAttachments; 
	subpass.description[0].pDepthStencilAttachment = &depthAttachment.reference;

	VkAttachmentReference inputReferences[3];
	inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	inputReferences[1] = { 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	inputReferences[2] = { 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };


	subpass.description[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.description[1].colorAttachmentCount = 1;
	subpass.description[1].pColorAttachments = &colorAttachemnt.reference;
	subpass.description[1].pDepthStencilAttachment = &depthAttachment.reference;
	subpass.description[1].inputAttachmentCount = 3;
	subpass.description[1].pInputAttachments = inputReferences;



	subpass.dependencies[0].srcSubpass =		VK_SUBPASS_EXTERNAL;
	subpass.dependencies[0].dstSubpass =		0;
	subpass.dependencies[0].srcStageMask =		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpass.dependencies[0].dstStageMask =		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass.dependencies[0].srcAccessMask =		VK_ACCESS_MEMORY_READ_BIT;
	subpass.dependencies[0].dstAccessMask =		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;
	subpass.dependencies[0].dependencyFlags=	VK_DEPENDENCY_BY_REGION_BIT;

	subpass.dependencies[1].srcSubpass = 0;		
	subpass.dependencies[1].dstSubpass = 1;		
	subpass.dependencies[1].srcStageMask =		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass.dependencies[1].dstStageMask =		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	subpass.dependencies[1].srcAccessMask =		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass.dependencies[1].dstAccessMask =		VK_ACCESS_SHADER_READ_BIT;
	subpass.dependencies[1].dependencyFlags	=   VK_DEPENDENCY_BY_REGION_BIT;

	subpass.dependencies[2].srcSubpass = 0;
	subpass.dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpass.dependencies[2].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	subpass.dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpass.dependencies[2].srcAccessMask = VK_ACCESS_SHADER_READ_BIT ;;
	subpass.dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpass.dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


	renderpassProperties.attachments[0] = colorAttachemnt;
	renderpassProperties.attachments[1] = albedoAttachment;
	renderpassProperties.attachments[2] = metallicRoughnessAttachment;
	renderpassProperties.attachments[3] = NormalsAttachment;
	renderpassProperties.attachments[4] = depthAttachment;

	gbufferRenderpass->properties = renderpassProperties;
	gbufferRenderpass->subpass = subpass;

	Vk_Functions::createRenderpass(device, *gbufferRenderpass.get());

	passes[gbufferRenderpass->getKey()] = std::move(gbufferRenderpass);


}
