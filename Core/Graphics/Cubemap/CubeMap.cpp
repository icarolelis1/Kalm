#include "CubeMap.h"

VK_Objects::PBuffer Vk_Functions::createCubeVertexBuffer(const VK_Objects::Device* device, VK_Objects::CommandPool* transientPool )
{

	struct vertex {
		glm::vec3 pos;
	};

	std::vector<float> vertices = {

		-1.0f, -1.0f, -1.0f, // bottom-left
		 1.0f,  1.0f, -1.0f, // top-right
		 1.0f, -1.0f, -1.0f, // bottom-right         
		 1.0f,  1.0f, -1.0f, // top-right
		-1.0f, -1.0f, -1.0f, // bottom-left
		-1.0f,  1.0f, -1.0f, // top-left
		// front face
		-1.0f, -1.0f,  1.0f, // bottom-left
		 1.0f, -1.0f,  1.0f, // bottom-right
		 1.0f,  1.0f,  1.0f, // top-right
		 1.0f,  1.0f,  1.0f, // top-right
		-1.0f,  1.0f,  1.0f, // top-left
		-1.0f, -1.0f,  1.0f, // bottom-left
		// left face
		-1.0f,  1.0f,  1.0f, // top-right
		-1.0f,  1.0f, -1.0f, // top-left
		-1.0f, -1.0f, -1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, // top-right
		// right face
		 1.0f,  1.0f,  1.0f, // top-left
		 1.0f, -1.0f, -1.0f, // bottom-right
		 1.0f,  1.0f, -1.0f, // top-right         
		 1.0f, -1.0f, -1.0f, // bottom-right
		 1.0f,  1.0f,  1.0f, // top-left
		 1.0f, -1.0f,  1.0f, // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f, // top-right
		 1.0f, -1.0f, -1.0f, // top-left
		 1.0f, -1.0f,  1.0f, // bottom-left
		 1.0f, -1.0f,  1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, // bottom-right
		-1.0f, -1.0f, -1.0f, // top-right
		// top face
		-1.0f,  1.0f, -1.0f, // top-left
		 1.0f,  1.0f , 1.0f, // bottom-right
		 1.0f,  1.0f, -1.0f, // top-right     
		 1.0f,  1.0f,  1.0f, // bottom-right
		-1.0f,  1.0f, -1.0f, // top-left
		-1.0f,  1.0f,  1.0f // bottom-left  
	};

	VkDeviceSize size = sizeof(float) * vertices.size();

	VkCommandBuffer cmd = transientPool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY)->getCommandBufferHandle();

	//VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties
	VK_Objects::Buffer stagingBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingBuffer.id = "staging-Vertex";

	void* data;
	vkMapMemory(device->getLogicalDevice(), stagingBuffer.getMemoryHandle(), 0, size, 0, &data);
	memcpy(data, vertices.data(), (size_t)size);
	vkUnmapMemory(device->getLogicalDevice(), stagingBuffer.getMemoryHandle());

	VK_Objects::PBuffer vertexBuffer =std::make_unique<VK_Objects::Buffer>(device, size, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	Vk_Functions::copyBuffer(cmd, stagingBuffer, *vertexBuffer.get(), size, device->getQueueHandle(VK_Objects::QUEUE_TYPE::TRANSFER));

	vkFreeCommandBuffers(device->getLogicalDevice(), transientPool->getPoolHanndle(), 1, &cmd);


	return std::move(vertexBuffer);
}

void Vk_Functions::convertEquirectangularImageToCubeMap(const VK_Objects::Device* device, const char* hdriImagePath, VK_Objects::CubeMap& cubeMap, VK_Objects::CommandPool* transientPool, VK_Objects::CommandPool* graphicsPool, VK_Objects::SDescriptorPoolManager poolManager)
{
	int dim = 1080;

	//HDRI image uses float format
	VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;


	int texWidth, texHeight, texChannels;
	float* pixels = stbi_loadf(hdriImagePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	VkDeviceSize imageSize;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image for path HDri Image: \n\n");
	}

	imageSize = texWidth * texHeight * 4 * sizeof(float);

	VK_Objects::Buffer stagingBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingBuffer.id = "xn";

	void* data;
	vkMapMemory(device->getLogicalDevice(), stagingBuffer.getMemoryHandle(), 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device->getLogicalDevice(), stagingBuffer.getMemoryHandle());

	free(pixels);

	//Image(const Device * device, uint32_t Width, uint32_t Height, ImageFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageCreateFlags flags, VkImageAspectFlags aspectFlags, uint32_t arrayLayers = 1, bool useMaxNumMips = 0);

	Vk_Functions::setImageLayout(*device, *transientPool, cubeMap.getVkImageHandlee(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, 0);

	VK_Objects::Image equirect(device, texWidth, texHeight, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);
	Vk_Functions::setImageLayout(*device, *transientPool, equirect.getVkImageHandle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 0);
	copyBufferToImage(stagingBuffer, equirect.getVkImageHandle(), *device, transientPool, texWidth, texHeight, 0, 1);

	Vk_Functions::setImageLayout(*device, *transientPool, equirect.getVkImageHandle(), format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 0);

	//Create renderpass

	VkExtent2D extent{ dim,dim };
	std::unique_ptr<VK_Objects::Renderpass> renderpass = std::make_unique<VK_Objects::Renderpass>(device, "CONVERT_EQUIRECTANGULAR_TO_CUBEMAP", extent);

	VK_Objects::RenderpassProperties renderpassProperties;

	//Single Attachment
	renderpassProperties.attachments.resize(1);

	VK_Objects::RenderAttachment cubeMapFaceAttachment;
	cubeMapFaceAttachment.description.format = format;
	cubeMapFaceAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	cubeMapFaceAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	cubeMapFaceAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	cubeMapFaceAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	cubeMapFaceAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	cubeMapFaceAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	cubeMapFaceAttachment.description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	cubeMapFaceAttachment.description.flags = 0;

	cubeMapFaceAttachment.reference.attachment = 0;
	cubeMapFaceAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	

	VK_Objects::Subpass subpass;
	subpass.description.resize(1);
	subpass.dependencies.resize(2);

	VkAttachmentReference colorReference = { cubeMapFaceAttachment.reference };

	subpass.description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.description[0].pColorAttachments = &colorReference;
	subpass.description[0].colorAttachmentCount = 1;;

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

	renderpassProperties.attachments[0] = cubeMapFaceAttachment;
	renderpass->properties = renderpassProperties;
	renderpass->subpass = subpass;

	Vk_Functions::createRenderpass(device, *renderpass.get());




	//Create  pipeline
	std::vector<VK_Objects::ShaderResource> equirectangularImageResource;
	equirectangularImageResource.resize(1);
	equirectangularImageResource[0].binding = static_cast<uint32_t>(0);
	equirectangularImageResource[0].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	equirectangularImageResource[0].type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;


	//Create shader resource and Allocate Descriptorsets 
	std::vector<VK_Objects::ShaderResource> resources{ equirectangularImageResource };
	//		DescriptorSetLayout(const VK_Objects::Device* _device, const std::vector<ShaderResource> _resources);

	VK_Objects::SDescriptorsetLayout descLayout = std::make_shared<VK_Objects::DescriptorSetLayout>(device, equirectangularImageResource);


	std::vector<std::shared_ptr<VK_Objects::DescriptorSetLayout>> descriptors;

	VK_Objects::Descriptorset descriptorset(device);

	poolManager;
	descriptorset = poolManager->allocateDescriptor(descLayout);


	descriptors.push_back(std::move(descLayout));

	std::unique_ptr<VK_Objects::Shader> vertex = std::make_unique< VK_Objects::Shader>(*device, VK_Objects::SHADER_TYPE::VERTEX_SHADER, Utils::readFile("Shaders\\equirectangular\\vert.spv"));
	std::unique_ptr<VK_Objects::Shader> fragment = std::make_unique< VK_Objects::Shader>(*device, VK_Objects::SHADER_TYPE::FRAGMENT_SHADER, Utils::readFile("Shaders\\equirectangular\\frag.spv"));



	std::vector<VkPushConstantRange> pushConstants;
	pushConstants.resize(1);
	struct Push {
		glm::mat4 view;
		glm::mat4 projection;

	}push;

	pushConstants[0].offset = 0;
	pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstants[0].size = sizeof(Push);

	std::unique_ptr<VK_Objects::PipelineLayout> layout = std::make_unique<VK_Objects::PipelineLayout>(*device, std::move(descriptors), pushConstants);

	VK_Objects::PipelineProperties pipelineInfo{};

	std::vector<VK_Objects::ATRIBUTES> atributes = { VK_Objects::ATRIBUTES::VEC3 };

	std::vector<std::vector<VK_Objects::ATRIBUTES>>att{ atributes };

	std::unique_ptr<VK_Objects::Pipeline> pipeline;

	pipelineInfo.atributes = att;
	pipelineInfo.colorAttachmentsCount = 1;
	pipelineInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
	pipelineInfo.dephTest = 0;
	pipelineInfo.depthBias = 0;
	pipelineInfo.rdpass = &renderpass->vk_renderpass;
	pipelineInfo.frontFaceClock = VK_FRONT_FACE_CLOCKWISE;
	pipelineInfo.vertexOffsets = { 0 };
	pipelineInfo.subpass = 0;

	pipeline = std::make_unique<VK_Objects::Pipeline>(*device, std::move(layout), std::move(vertex), std::move(fragment), pipelineInfo);
	pipeline->id = "CONVERT_EQUIRECT_TO_CUBEMAP";


	VkFramebuffer framebuffer;
	//Create Offscreen frameBuffer
	//(device, extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1,0 );
	VK_Objects::Image offscreen(device, 1080, 1080, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT,1,0);



	VkFramebufferCreateInfo fbufCreateInfo{};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.renderPass = renderpass->vk_renderpass;
	fbufCreateInfo.attachmentCount = 1;
	fbufCreateInfo.pAttachments = offscreen.getVkImageViewHandle();
	fbufCreateInfo.width =dim;;
	fbufCreateInfo.height = dim;
	fbufCreateInfo.layers = 1;

	VkResult r = vkCreateFramebuffer(device->getLogicalDevice(), &fbufCreateInfo, device->getAllocator(), &framebuffer);

	if (r != VK_SUCCESS) {
		std::cout << "    Failed to create FrameBuffer\n";
	}
	//Vk_Functions::setImageLayout(*device, transientPool, offscreen.getVkImageHandle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


	VkSampler sampler;
	Vk_Functions::createSampler(device, sampler);

	std::vector<VkDescriptorBufferInfo> bufferInfos;


	std::vector<VkDescriptorImageInfo> imageInfo;
	imageInfo.resize(1);

	imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo[0].imageView = *equirect.getVkImageViewHandle();
	imageInfo[0].sampler = sampler;
	//void VK_Objects::Descriptorset::updateDescriptorset(int &startIndex, std::vector<VkDescriptorImageInfo>& imageInfos, bool isInputAttachment)

	descriptorset.updateDescriptorset(0,imageInfo,false);
	
	VK_Objects::PBuffer vertexBuffer;
	vertexBuffer = createCubeVertexBuffer(device, transientPool);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[6] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	for (int i = 0; i < 6; i++) {



		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderpass->vk_renderpass;
		renderPassInfo.framebuffer  = framebuffer;
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent.width = 1080;
		renderPassInfo.renderArea.extent.height = 1080;

		VkClearValue clearValues = {};
		clearValues.color = { 0.0f, .0f, 1.0f, 0.f };

		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearValues;

		VK_Objects::CommandBuffer commandBuffer = *graphicsPool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		Vk_Functions::beginCommandBuffer(commandBuffer.getCommandBufferHandle());

		vkCmdBeginRenderPass(commandBuffer.getCommandBufferHandle(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = {};
		viewport.height = 1080;
		viewport.width = 1080;

		viewport.maxDepth = 1.0f;

		VkRect2D rect = {};
		rect.extent.width = 1080;
		rect.extent.height = 1080;
		rect.offset = { 0,0 };
		vkCmdSetViewport(commandBuffer.getCommandBufferHandle(), 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer.getCommandBufferHandle(), 0, 1, &rect);

		VkDeviceSize offsets[1] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer.getCommandBufferHandle(), 0, 1, &vertexBuffer->getBufferHandle(), offsets);

		std::array<VkDescriptorSet, 1>sets = { descriptorset.getDescriptorSetHandle() };
		vkCmdBindPipeline(commandBuffer.getCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineHandle());
		vkCmdBindDescriptorSets(commandBuffer.getCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayoutHandle()->getHandle(), 0, 1, sets.data(), 0, nullptr);

		push.view = captureViews[i];
		push.projection = captureProjection;

		vkCmdPushConstants(
			commandBuffer.getCommandBufferHandle(),
			pipeline->getPipelineLayoutHandle()->getHandle(),
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(push),
			&push);

		vkCmdDraw(commandBuffer.getCommandBufferHandle(), 36, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer.getCommandBufferHandle());

		endCommandBuffer(commandBuffer.getCommandBufferHandle());

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer.getCommandBufferHandle();
		vkQueueSubmit(device->getQueueHandle(VK_Objects::QUEUE_TYPE::GRAPHICS), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device->getQueueHandle(VK_Objects::QUEUE_TYPE::GRAPHICS));
		vkFreeCommandBuffers(device->getLogicalDevice(), graphicsPool->getPoolHanndle(), 1, &commandBuffer.getCommandBufferHandle());
	
		//Transfering operations

		commandBuffer = *transientPool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		Vk_Functions::beginCommandBuffer(commandBuffer.getCommandBufferHandle());

		VkImageCopy copyRegion = {};

		copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.baseArrayLayer = 0;
		copyRegion.srcSubresource.mipLevel = 0;
		copyRegion.srcSubresource.layerCount = 1;
		copyRegion.srcOffset = { 0, 0, 0 };

		copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.baseArrayLayer = i;
		copyRegion.dstSubresource.mipLevel = 0;
		copyRegion.dstSubresource.layerCount = 1;
		copyRegion.dstOffset = { 0, 0, 0 };

		copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
		copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
		copyRegion.extent.depth = 1;

		setImageLayout(*device, *transientPool, offscreen.getVkImageHandle(), format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 0);

		vkCmdCopyImage(
			commandBuffer.getCommandBufferHandle(),
			offscreen.getVkImageHandle(),
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			cubeMap.getVkImageHandlee(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&copyRegion);

		//setImageLayout(*device,transientPool,offscreen.getVkImageHandle(), format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 0);

		endCommandBuffer(commandBuffer.getCommandBufferHandle());


	    submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer.getCommandBufferHandle();
		vkQueueSubmit(device->getQueueHandle(VK_Objects::QUEUE_TYPE::TRANSFER), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device->getQueueHandle(VK_Objects::QUEUE_TYPE::TRANSFER));
		vkFreeCommandBuffers(device->getLogicalDevice(), transientPool->getPoolHanndle(), 1, &commandBuffer.getCommandBufferHandle());

	}
	setImageLayout(*device, *transientPool, cubeMap.getVkImageHandlee(), format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6, 0);



	vkDestroySampler(device->getLogicalDevice(), sampler, device->getAllocator());
	renderpass.reset();
	vkDestroyFramebuffer(device->getLogicalDevice(), framebuffer, device->getAllocator());
	vertexBuffer.reset();
	pipeline.reset();

}

void Vk_Functions::filterEnviromentMap(const VK_Objects::Device* device, VK_Objects::CubeMap& skybox, VK_Objects::CubeMap& envMap, VK_Objects::CommandPool& transientPool, VK_Objects::CommandPool& graphicsPool, VK_Objects::SDescriptorPoolManager poolManager)
{
	int dim = 512;

	//HDRI image uses float format
	VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
	const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

	VkExtent2D extent{ dim,dim };
	std::unique_ptr<VK_Objects::Renderpass> renderpass = std::make_unique<VK_Objects::Renderpass>(device, "IBL_FILTERED_ENVMAP", extent);

	VK_Objects::RenderpassProperties renderpassProperties;

	//Single Attachment
	renderpassProperties.attachments.resize(1);

	VK_Objects::RenderAttachment cubeMapFaceAttachment;
	cubeMapFaceAttachment.description.format = format;
	cubeMapFaceAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	cubeMapFaceAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	cubeMapFaceAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	cubeMapFaceAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	cubeMapFaceAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	cubeMapFaceAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	cubeMapFaceAttachment.description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	cubeMapFaceAttachment.description.flags = 0;

	cubeMapFaceAttachment.reference.attachment = 0;
	cubeMapFaceAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	VK_Objects::Subpass subpass;
	subpass.description.resize(1);
	subpass.dependencies.resize(2);

	VkAttachmentReference colorReference = { cubeMapFaceAttachment.reference };

	subpass.description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.description[0].pColorAttachments = &colorReference;
	subpass.description[0].colorAttachmentCount = 1;;

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

	renderpassProperties.attachments[0] = cubeMapFaceAttachment;
	renderpass->properties = renderpassProperties;
	renderpass->subpass = subpass;

	Vk_Functions::createRenderpass(device, *renderpass.get());




	//Create  pipeline
	std::vector<VK_Objects::ShaderResource> skyboxResource;
	skyboxResource.resize(1);
	skyboxResource[0].binding = static_cast<uint32_t>(0);
	skyboxResource[0].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	skyboxResource[0].type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;


	//Create shader resource and Allocate Descriptorsets 
	std::vector<VK_Objects::ShaderResource> resources{ skyboxResource };
	//		DescriptorSetLayout(const VK_Objects::Device* _device, const std::vector<ShaderResource> _resources);

	VK_Objects::SDescriptorsetLayout descLayout = std::make_shared<VK_Objects::DescriptorSetLayout>(device, skyboxResource);


	std::vector<std::shared_ptr<VK_Objects::DescriptorSetLayout>> descriptors;

	VK_Objects::Descriptorset descriptorset(device);

	descriptorset = poolManager->allocateDescriptor(descLayout);

	descriptors.push_back(std::move(descLayout));

	std::unique_ptr<VK_Objects::Shader> vertex = std::make_unique< VK_Objects::Shader>(*device, VK_Objects::SHADER_TYPE::VERTEX_SHADER, Utils::readFile("Shaders\\iblEnvMap\\vert.spv"));
	std::unique_ptr<VK_Objects::Shader> fragment = std::make_unique< VK_Objects::Shader>(*device, VK_Objects::SHADER_TYPE::FRAGMENT_SHADER, Utils::readFile("Shaders\\iblEnvMap\\frag.spv"));

	std::vector<VkPushConstantRange> pushConstants;
	pushConstants.resize(1);

	struct Push {
		glm::mat4 view;
		glm::mat4 projection;
		float roughness;


	}push;

	pushConstants[0].offset = 0;
	pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstants[0].size = sizeof(Push);

	std::unique_ptr<VK_Objects::PipelineLayout> layout = std::make_unique<VK_Objects::PipelineLayout>(*device, std::move(descriptors), pushConstants);

	VK_Objects::PipelineProperties pipelineInfo{};

	std::vector<VK_Objects::ATRIBUTES> atributes = { VK_Objects::ATRIBUTES::VEC3 };

	std::vector<std::vector<VK_Objects::ATRIBUTES>>att{ atributes };

	std::unique_ptr<VK_Objects::Pipeline> pipeline;

	pipelineInfo.atributes = att;
	pipelineInfo.colorAttachmentsCount = 1;
	pipelineInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	pipelineInfo.dephTest = 0;
	pipelineInfo.depthBias = 0;
	pipelineInfo.rdpass = &renderpass->vk_renderpass;
	pipelineInfo.frontFaceClock = VK_FRONT_FACE_CLOCKWISE;
	pipelineInfo.vertexOffsets = { 0 };
	pipelineInfo.subpass = 0;

	pipeline = std::make_unique<VK_Objects::Pipeline>(*device, std::move(layout), std::move(vertex), std::move(fragment), pipelineInfo);


	VkFramebuffer framebuffer;
	//Create Offscreen frameBuffer
	//(device, extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1,0 );
	VK_Objects::Image offscreen(device, dim, dim, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);



	VkFramebufferCreateInfo fbufCreateInfo{};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.renderPass = renderpass->vk_renderpass;
	fbufCreateInfo.attachmentCount = 1;
	fbufCreateInfo.pAttachments = offscreen.getVkImageViewHandle();
	fbufCreateInfo.width = dim;;
	fbufCreateInfo.height = dim;
	fbufCreateInfo.layers = 1;

	VkResult r = vkCreateFramebuffer(device->getLogicalDevice(), &fbufCreateInfo, device->getAllocator(), &framebuffer);

	if (r != VK_SUCCESS) {
		std::cout << "    Failed to create FrameBuffer\n";
	}
	//Vk_Functions::setImageLayout(*device, transientPool, offscreen.getVkImageHandle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


	VkSampler sampler;
	VkSamplerCreateInfo samplerCI = initializers::samplerCreateInfo();
	samplerCI.magFilter = VK_FILTER_LINEAR;
	samplerCI.minFilter = VK_FILTER_LINEAR;
	samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = 1;
	samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	if (vkCreateSampler(device->getLogicalDevice(), &samplerCI, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	std::vector<VkDescriptorBufferInfo> bufferInfos;


	std::vector<VkDescriptorImageInfo> imageInfo;
	imageInfo.resize(1);

	imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo[0].imageView = skybox.getVkViewHandle();
	imageInfo[0].sampler = sampler;
	//void VK_Objects::Descriptorset::updateDescriptorset(int &startIndex, std::vector<VkDescriptorImageInfo>& imageInfos, bool isInputAttachment)

	descriptorset.updateDescriptorset(0, imageInfo, false);

	VK_Objects::PBuffer vertexBuffer;
	vertexBuffer = createCubeVertexBuffer(device, &transientPool);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[6] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	Vk_Functions::setImageLayout(*device, transientPool, envMap.getVkImageHandlee(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, 0, numMips);
	VkViewport viewport = initializers::viewport((float)dim, (float)dim, 0.0f, 1.0f);
	VkRect2D scissor = initializers::rect2D(dim, dim, 0, 0);


	for (uint32_t m = 0; m < numMips; m++) {
		push.roughness = (float)m / (float)(numMips - 1);
		for (int f = 0; f < 6; f++) {
			VK_Objects::CommandBuffer commandBuffer = *graphicsPool.requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			Vk_Functions::beginCommandBuffer(commandBuffer.getCommandBufferHandle());

			viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
			viewport.height = static_cast<float>(dim * std::pow(0.5f, m));
			vkCmdSetViewport(commandBuffer.getCommandBufferHandle(), 0, 1, &viewport);

			VkRect2D rect;
			rect.extent.width = viewport.width;
			rect.extent.height = viewport.height;
			rect.offset = { 0,0 };
			vkCmdSetScissor(commandBuffer.getCommandBufferHandle(), 0, 1, &rect);


			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderpass->vk_renderpass;
			renderPassInfo.framebuffer = framebuffer;
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent.width = dim;
			renderPassInfo.renderArea.extent.height = dim;

			VkClearValue clearValues = {};
			clearValues.color = { 0.0f, .0f, 1.0f, 0.f };

			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearValues;

			vkCmdBeginRenderPass(commandBuffer.getCommandBufferHandle(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			push.view = captureViews[f];


			vkCmdPushConstants(commandBuffer.getCommandBufferHandle(), pipeline->getPipelineLayoutHandle()->getHandle(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Push), &push);

			vkCmdBindPipeline(commandBuffer.getCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineHandle());
			vkCmdBindDescriptorSets(commandBuffer.getCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayoutHandle()->getHandle(), 0, 1, &descriptorset.getDescriptorSetHandle(), 0, NULL);

			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer.getCommandBufferHandle(), 0, 1, &vertexBuffer->getBufferHandle(), offsets);
			vkCmdDraw(commandBuffer.getCommandBufferHandle(), 36, 1, 0, 0);

			vkCmdEndRenderPass(commandBuffer.getCommandBufferHandle());
			Vk_Functions::endCommandBuffer(commandBuffer.getCommandBufferHandle());


			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer.getCommandBufferHandle();
			vkQueueSubmit(device->getQueueHandle(VK_Objects::QUEUE_TYPE::GRAPHICS), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(device->getQueueHandle(VK_Objects::QUEUE_TYPE::GRAPHICS));
			vkFreeCommandBuffers(device->getLogicalDevice(), graphicsPool.getPoolHanndle(), 1, &commandBuffer.getCommandBufferHandle());

			//setImageLayout(de, *device, commandPool, offscreen.image, format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 0, cmdBuffer);
			Vk_Functions::setImageLayout(*device, transientPool, offscreen.getVkImageHandle(), format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 0, 1);

		   commandBuffer = *transientPool.requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			Vk_Functions::beginCommandBuffer(commandBuffer.getCommandBufferHandle());
			VkImageCopy copyRegion = {};

			copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.srcSubresource.baseArrayLayer = 0;
			copyRegion.srcSubresource.mipLevel = 0;
			copyRegion.srcSubresource.layerCount = 1;
			copyRegion.srcOffset = { 0, 0, 0 };

			copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.dstSubresource.baseArrayLayer = f;
			copyRegion.dstSubresource.mipLevel = m;
			copyRegion.dstSubresource.layerCount = 1;
			copyRegion.dstOffset = { 0, 0, 0 };

			copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
			copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
			copyRegion.extent.depth = 1;

			vkCmdCopyImage(
				commandBuffer.getCommandBufferHandle(),
				offscreen.getVkImageHandle(),
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				envMap.getVkImageHandlee(),
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&copyRegion);
			Vk_Functions::endCommandBuffer(commandBuffer.getCommandBufferHandle());

			submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer.getCommandBufferHandle();
			vkQueueSubmit(device->getQueueHandle(VK_Objects::QUEUE_TYPE::TRANSFER), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(device->getQueueHandle(VK_Objects::QUEUE_TYPE::TRANSFER));
			vkFreeCommandBuffers(device->getLogicalDevice(), transientPool.getPoolHanndle(), 1, &commandBuffer.getCommandBufferHandle());

			Vk_Functions::setImageLayout(*device, transientPool, offscreen.getVkImageHandle(), format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 0, 1);
		}
	}

	Vk_Functions::setImageLayout(*device, transientPool, envMap.getVkImageHandlee(), format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6, 0, numMips);



	vkDestroySampler(device->getLogicalDevice(), sampler, device->getAllocator());
	renderpass.reset();
	vkDestroyFramebuffer(device->getLogicalDevice(), framebuffer, device->getAllocator());
	vertexBuffer.reset();
	pipeline.reset();

}

void Vk_Functions::generatBRDFLut(const VK_Objects::Device* device, VK_Objects::Image& brdfLut, VK_Objects::CommandPool& transientPool, VK_Objects::CommandPool& graphicsPool, VK_Objects::SDescriptorPoolManager poolManager)
{
	uint32_t dim = 512;
	const VkFormat format = VK_FORMAT_R16G16_SFLOAT;	// R16G16 is supported pretty much everywhere

	Vk_Functions::setImageLayout(*device, transientPool, brdfLut.getVkImageHandle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 0);

	//Create renderpass

	VkExtent2D extent{ dim,dim };
	std::unique_ptr<VK_Objects::Renderpass> renderpass = std::make_unique<VK_Objects::Renderpass>(device, "CONVERT_EQUIRECTANGULAR_TO_CUBEMAP", extent);

	VK_Objects::RenderpassProperties renderpassProperties;

	//Single Attachment
	renderpassProperties.attachments.resize(1);

	VK_Objects::RenderAttachment cubeMapFaceAttachment;
	cubeMapFaceAttachment.description.format = format;
	cubeMapFaceAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	cubeMapFaceAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	cubeMapFaceAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	cubeMapFaceAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	cubeMapFaceAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	cubeMapFaceAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	cubeMapFaceAttachment.description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	cubeMapFaceAttachment.description.flags = 0;

	cubeMapFaceAttachment.reference.attachment = 0;
	cubeMapFaceAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	VK_Objects::Subpass subpass;
	subpass.description.resize(1);
	subpass.dependencies.resize(2);

	VkAttachmentReference colorReference = { cubeMapFaceAttachment.reference };

	subpass.description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.description[0].pColorAttachments = &colorReference;
	subpass.description[0].colorAttachmentCount = 1;;

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

	renderpassProperties.attachments[0] = cubeMapFaceAttachment;
	renderpass->properties = renderpassProperties;
	renderpass->subpass = subpass;

	Vk_Functions::createRenderpass(device, *renderpass.get());



	std::vector<VK_Objects::ShaderResource> empty;;



	//Create shader resource and Allocate Descriptorsets 
	std::vector<VK_Objects::ShaderResource> resources{ empty };
	//		DescriptorSetLayout(const VK_Objects::Device* _device, const std::vector<ShaderResource> _resources);

	VK_Objects::SDescriptorsetLayout descLayout = std::make_shared<VK_Objects::DescriptorSetLayout>(device, empty);


	std::vector<std::shared_ptr<VK_Objects::DescriptorSetLayout>> descriptors;

	descriptors.push_back(std::move(descLayout));

	std::unique_ptr<VK_Objects::Shader> vertex = std::make_unique< VK_Objects::Shader>(*device, VK_Objects::SHADER_TYPE::VERTEX_SHADER, Utils::readFile("Shaders\\brdfLut\\vert.spv"));
	std::unique_ptr<VK_Objects::Shader> fragment = std::make_unique< VK_Objects::Shader>(*device, VK_Objects::SHADER_TYPE::FRAGMENT_SHADER, Utils::readFile("Shaders\\brdfLut\\frag.spv"));



	std::vector<VkPushConstantRange> pushConstants;


	std::unique_ptr<VK_Objects::PipelineLayout> layout = std::make_unique<VK_Objects::PipelineLayout>(*device, std::move(descriptors), pushConstants);

	VK_Objects::PipelineProperties pipelineInfo{};

	std::vector<VK_Objects::ATRIBUTES> atributes = { VK_Objects::ATRIBUTES::NONE };

	std::vector<std::vector<VK_Objects::ATRIBUTES>>att{ atributes };

	std::unique_ptr<VK_Objects::Pipeline> pipeline;

	pipelineInfo.atributes = att;
	pipelineInfo.colorAttachmentsCount = 1;
	pipelineInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	pipelineInfo.dephTest = 0;
	pipelineInfo.depthBias = 0;
	pipelineInfo.rdpass = &renderpass->vk_renderpass;
	pipelineInfo.frontFaceClock = VK_FRONT_FACE_CLOCKWISE;
	pipelineInfo.vertexOffsets = { 0 };
	pipelineInfo.subpass = 0;
	pipelineInfo.vertexBindingCount = 0;
	pipeline = std::make_unique<VK_Objects::Pipeline>(*device, std::move(layout), std::move(vertex), std::move(fragment), pipelineInfo);
	pipeline->id = "CONVERT_EQUIRECT_TO_CUBEMAP";


	VkFramebuffer framebuffer;
	//Create Offscreen frameBuffer
	//(device, extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1,0 );
	VK_Objects::Image offscreen(device, dim, dim, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);



	VkFramebufferCreateInfo fbufCreateInfo{};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.renderPass = renderpass->vk_renderpass;
	fbufCreateInfo.attachmentCount = 1;
	fbufCreateInfo.pAttachments = offscreen.getVkImageViewHandle();
	fbufCreateInfo.width = dim;;
	fbufCreateInfo.height = dim;
	fbufCreateInfo.layers = 1;

	VkResult r = vkCreateFramebuffer(device->getLogicalDevice(), &fbufCreateInfo, device->getAllocator(), &framebuffer);

	if (r != VK_SUCCESS) {
		std::cout << "    Failed to create FrameBuffer\n";
	}
	//Vk_Functions::setImageLayout(*device, transientPool, offscreen.getVkImageHandle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


	VkSampler sampler;
	Vk_Functions::createSampler(device, sampler);

	std::vector<VkDescriptorBufferInfo> bufferInfos;


	std::vector<VkDescriptorImageInfo> imageInfo;

	//void VK_Objects::Descriptorset::updateDescriptorset(int &startIndex, std::vector<VkDescriptorImageInfo>& imageInfos, bool isInputAttachment)




		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderpass->vk_renderpass;
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent.width = dim;
		renderPassInfo.renderArea.extent.height = dim;

		VkClearValue clearValues = {};
		clearValues.color = { 0.0f, .0f, 1.0f, 0.f };

		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearValues;

		VK_Objects::CommandBuffer commandBuffer = *graphicsPool.requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		Vk_Functions::beginCommandBuffer(commandBuffer.getCommandBufferHandle());

		vkCmdBeginRenderPass(commandBuffer.getCommandBufferHandle(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = {};
		viewport.height = dim;
		viewport.width = dim;

		viewport.maxDepth = 1.0f;

		VkRect2D rect = {};
		rect.extent.width = dim;
		rect.extent.height = dim;
		rect.offset = { 0,0 };
		vkCmdSetViewport(commandBuffer.getCommandBufferHandle(), 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer.getCommandBufferHandle(), 0, 1, &rect);

		VkDeviceSize offsets[1] = { 0 };

		vkCmdBindPipeline(commandBuffer.getCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineHandle());
		//vkCmdBindDescriptorSets(commandBuffer.getCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayoutHandle()->getHandle(), 0,0, VK_NULL_HANDLE, 0, nullptr);


		vkCmdDraw(commandBuffer.getCommandBufferHandle(), 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer.getCommandBufferHandle());

		endCommandBuffer(commandBuffer.getCommandBufferHandle());

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer.getCommandBufferHandle();
		vkQueueSubmit(device->getQueueHandle(VK_Objects::QUEUE_TYPE::GRAPHICS), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device->getQueueHandle(VK_Objects::QUEUE_TYPE::GRAPHICS));
		vkFreeCommandBuffers(device->getLogicalDevice(), graphicsPool.getPoolHanndle(), 1, &commandBuffer.getCommandBufferHandle());

		//Transfering operations

		commandBuffer = *transientPool.requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		Vk_Functions::beginCommandBuffer(commandBuffer.getCommandBufferHandle());

		VkImageCopy copyRegion = {};

		copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.baseArrayLayer = 0;
		copyRegion.srcSubresource.mipLevel = 0;
		copyRegion.srcSubresource.layerCount = 1;
		copyRegion.srcOffset = { 0, 0, 0 };

		copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.baseArrayLayer = 0;
		copyRegion.dstSubresource.mipLevel = 0;
		copyRegion.dstSubresource.layerCount = 1;
		copyRegion.dstOffset = { 0, 0, 0 };

		copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
		copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
		copyRegion.extent.depth = 1;

		setImageLayout(*device, transientPool, offscreen.getVkImageHandle(), format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 0);

		vkCmdCopyImage(
			commandBuffer.getCommandBufferHandle(),
			offscreen.getVkImageHandle(),
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			brdfLut.getVkImageHandle(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&copyRegion);

		//setImageLayout(*device,transientPool,offscreen.getVkImageHandle(), format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 0);

		endCommandBuffer(commandBuffer.getCommandBufferHandle());


		submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer.getCommandBufferHandle();
		vkQueueSubmit(device->getQueueHandle(VK_Objects::QUEUE_TYPE::TRANSFER), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device->getQueueHandle(VK_Objects::QUEUE_TYPE::TRANSFER));
		vkFreeCommandBuffers(device->getLogicalDevice(), transientPool.getPoolHanndle(), 1, &commandBuffer.getCommandBufferHandle());

	
	setImageLayout(*device, transientPool, brdfLut.getVkImageHandle(), format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 0);



	vkDestroySampler(device->getLogicalDevice(), sampler, device->getAllocator());
	renderpass.reset();
	vkDestroyFramebuffer(device->getLogicalDevice(), framebuffer, device->getAllocator());
	pipeline.reset();
}

VK_Objects::CubeMap::CubeMap(const VK_Objects::Device* _device, VkFormat format, VkMemoryPropertyFlags properties, int dim, int numMips) : device(_device)
{
	VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent.width = dim;
	imageCreateInfo.extent.height = dim;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = numMips;
	imageCreateInfo.arrayLayers = 6;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;


	vkCreateImage(device->getLogicalDevice(), &imageCreateInfo, device->getAllocator(), &vk_image);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device->getLogicalDevice(), vk_image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = Vk_Functions::findMemoryType(device->getPhysicalDevice(), memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device->getLogicalDevice(), &allocInfo, nullptr, &vk_memory) != VK_SUCCESS) {
		throw std::runtime_error("    Failed to allocate image memory!\n");
	}

	vkBindImageMemory(device->getLogicalDevice(), vk_image, vk_memory, 0);


	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewInfo.format = format;
	viewInfo.subresourceRange = {};
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = numMips;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 6;
	viewInfo.image = vk_image;

	if ((vkCreateImageView(device->getLogicalDevice(), &viewInfo, device->getAllocator(), &vk_view)) != VK_SUCCESS) {
		std::cout << "Failed to create SkyboxImageView\n";
	}
}

VkImage& VK_Objects::CubeMap::getVkImageHandlee()
{
	return vk_image;
}

VkImageView& VK_Objects::CubeMap::getVkViewHandle()
{
	return vk_view;
}

VK_Objects::CubeMap::~CubeMap()
{
	vkDestroyImage(device->getLogicalDevice(), vk_image, device->getAllocator());
	vkDestroyImageView(device->getLogicalDevice(), vk_view, device->getAllocator());
	vkFreeMemory(device->getLogicalDevice(), vk_memory, device->getAllocator());
}
