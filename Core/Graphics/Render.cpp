#include "Render.h"
void* alignedAlloc(size_t size, size_t alignment)
{
	void* data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
	data = _aligned_malloc(size, alignment);
#else
	int res = posix_memalign(&data, alignment, size);
	if (res != 0)
		data = nullptr;
#endif
	return data;
}

void alignedFree(void* data)
{
#if	defined(_MSC_VER) || defined(__MINGW32__)
	_aligned_free(data);
#else
	free(data);
#endif
}

Render::Render()
{
	nearFar = glm::vec2(.1f, 115.f);
	dist = 115;
}

void Render::initiateResources(Utils::WindowHandler* windowHandler, uint32_t WIDTH, uint32_t HEIGHT)
{
	window = windowHandler;

	// Instance Initialization
	createInstance();
	//Debuger Initialization
	createDebuger();
	//Creation of surface to be used in Window
	createSurface(windowHandler);
	//Prepation of the logical and the physical Device. Surface is required to query proper capabilities.
	prepareDevice(surface);
	//Get the queueFamily configurations
	VK_Objects::QueueSharingMode queueSharingMode = device.getQueueSharingMode();
	VK_Objects::ImageFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	//Setup the swapChain
	swapChain.prepareSwapChain(WIDTH, HEIGHT, device, &surface, format, windowHandler, queueSharingMode);
	//Create First renderpass;
	createRenderpass();
	framebuffersManager = std::make_unique<FramebufferManagement>(&device, &swapChain, renderpass->passes);
	createPipeline();
	createCommandPools();
	createScene();
	separateSceneObjects(mainSCene.sceneGraph.root);
	createMaterials();
	createDynamicUniformBuffers();
	if(UI_RENDER)
	createImGuiInterface();
	createSamplers();
	createEnvMaps();
	AllocateCommonDescriptorsSets();
	setqueryPoolStatistics();
	createRenderContexts();
}

//Creation of Instance
void Render::createInstance()
{

	VkInstanceCreateInfo instanceInfo{};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	//AplicationInfo;
	VkApplicationInfo appInfo{};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Kalm";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Kalm Render";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;



	//Instance
	instanceInfo.pApplicationInfo = &appInfo;

	//Extensions
	auto extensions = debuger.getExtension(DEBUG_);
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceInfo.ppEnabledExtensionNames = extensions.data();



	instanceInfo.enabledLayerCount = 0;
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	auto layers = debuger.getValidationLayers();

	/*for (auto layer : layers) {
		std::cout << layer << std::endl;
	}*/

	if (DEBUG_) {

		//Layers
		instanceInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		instanceInfo.ppEnabledLayerNames = layers.data();

		debuger.populateDebugMessengerCreateInfo(debugCreateInfo);
		instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

	}

	vkCreateInstance(&instanceInfo, device.getAllocator(), &instance.vk_Instance);

}

//Setup Debuger 
void Render::createDebuger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	debuger.setDebugerMessenger(createInfo, instance);
}

void Render::createSurface(Utils::WindowHandler* window)
{
	w = window;
	glfwCreateWindowSurface(instance.vk_Instance, w, nullptr, &surface);
}

void Render::prepareDevice(VK_Objects::Surface surface)
{
	device.choosePhysicalDevice(instance, surface);
	device.createLogicalDevice();
}

void Render::createRenderpass()
{
	renderpass = std::make_unique<Game::RenderpassManager>(&device, &swapChain, swapChain.getExtent());

}

void Render::createEnvMaps()
{
	//SKYBOX AND ENVIROMENT 
	cubeMap = std::make_unique<VK_Objects::CubeMap>(&device, VK_FORMAT_R32G32B32A32_SFLOAT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1080, 1);
	Vk_Functions::convertEquirectangularImageToCubeMap(&device, "Assets\\skyboxes\\Ice_Lake\\Ice_Lake\\Ice_Lake_Env.hdr", *cubeMap.get(), transferPool.get(), graphicsPool.get(), poolManager);
	const uint32_t numMips = static_cast<uint32_t>(floor(log2(512))) + 1;

	envMAp = std::make_unique<VK_Objects::CubeMap>(&device, VK_FORMAT_R32G32B32A32_SFLOAT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 512, numMips);
	Vk_Functions::filterEnviromentMap(&device, *cubeMap.get(), *envMAp.get(), *transferPool.get(), *graphicsPool.get(), poolManager);

	brdfLut = std::make_unique<VK_Objects::Image>(&device, 512, 512, VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0);
	Vk_Functions::generatBRDFLut(&device, *brdfLut.get(), *transferPool.get(), *graphicsPool.get(), poolManager);

	irradianceMap = std::make_unique<VK_Objects::CubeMap>(&device, VK_FORMAT_R32G32B32A32_SFLOAT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1080, 1);
	Vk_Functions::convertEquirectangularImageToCubeMap(&device, "Assets\\skyboxes\\Ice_Lake\\Ice_Lake\\Ice_Lake_Env.hdr", *irradianceMap.get(), transferPool.get(), graphicsPool.get(), poolManager);

}
void Render::AllocateCommonDescriptorsSets()
{

	uint32_t n = swapChain.getNumberOfImages();

	//Create Resources
	viewProjectionBuffers.clear();
	lightUniformBuffers.clear();
	lightProjectionUniformBuffers.clear();

	viewProjectionBuffers.resize(n);
	lightUniformBuffers.resize(n);
	lightProjectionUniformBuffers.resize(n);
	int index = 0;



	for (int i = 0; i < viewProjectionBuffers.size(); i++) {

		viewProjectionBuffers[i] = std::make_shared<VK_Objects::Buffer>(&device, sizeof(VP), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		lightUniformBuffers[i] = std::make_shared<VK_Objects::Buffer>(&device, sizeof(LightUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		lightProjectionUniformBuffers[i] = std::make_shared<VK_Objects::Buffer>(&device, sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		std::vector<VkDescriptorBufferInfo> bufferInfos;

		bufferInfos.resize(1);
		bufferInfos[0].buffer = viewProjectionBuffers[i]->getBufferHandle();
		bufferInfos[0].offset = static_cast<uint32_t>(0);
		bufferInfos[0].range = sizeof(VP);

		std::vector<VkDescriptorImageInfo> imageInfo;
		globalData_Descriptorsets[index].updateDescriptorset(bufferInfos, imageInfo);

		bufferInfos[0].buffer = lightUniformBuffers[i]->getBufferHandle();
		bufferInfos[0].offset = static_cast<uint32_t>(0);
		bufferInfos[0].range = sizeof(LightUniform);

		light_Descriptorsets[index].updateDescriptorset(bufferInfos, imageInfo);

		bufferInfos[0].buffer = modelBuffers[i]->getBufferHandle();
		bufferInfos[0].offset = static_cast<uint32_t>(0);
		bufferInfos[0].range = dynamicAlignment;

		imageInfo.clear();
		modelMatrix_Descriptorsets[index].updateDescriptorset(bufferInfos, imageInfo, 1);

		bufferInfos[0].buffer = lightProjectionUniformBuffers[i]->getBufferHandle();
		bufferInfos[0].offset = static_cast<uint32_t>(0);
		bufferInfos[0].range = sizeof(glm::mat4);

		lightProjection_Descriptorset[index].updateDescriptorset(bufferInfos, imageInfo);
		index++;

	}

	index = 0;

	for (int i = 0; i < deferredShading_Descriptorsets.size(); i++) {

		std::vector<VkDescriptorBufferInfo> bufferInfos;


		std::vector<VkDescriptorImageInfo> imageInfo;
		imageInfo.resize(4);

		imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[0].imageView = *framebuffersManager->g_bufferImages["ALBEDO"]->getVkImageViewHandle();
		imageInfo[0].sampler = sampler_Testing;

		imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[1].imageView = *framebuffersManager->g_bufferImages["METALLICROUGHNESS"]->getVkImageViewHandle();
		imageInfo[1].sampler = sampler_Testing;

		imageInfo[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[2].imageView = *framebuffersManager->g_bufferImages["NORMALS"]->getVkImageViewHandle();
		imageInfo[2].sampler = sampler_Testing;

		imageInfo[3].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[3].imageView = *framebuffersManager->g_bufferImages["EMISSION"]->getVkImageViewHandle();
		imageInfo[3].sampler = sampler_Testing;

		deferredShading_Descriptorsets[i].updateDescriptorset(bufferInfos, imageInfo);


		imageInfo.resize(1);
		imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[0].imageView = *framebuffersManager->deferreLighting_Images["DEFERRED_BRIGHTNESS_ATTACHMENT"]->getVkImageViewHandle();
		imageInfo[0].sampler = sampler_Testing;


		verticalBlur_Descriptorsets[i].updateDescriptorset(bufferInfos, imageInfo);

		imageInfo.resize(2);
		imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[0].imageView = *framebuffersManager->deferreLighting_Images["DEFERRED_LIGHTING_ATTACHMENT"]->getVkImageViewHandle();
		imageInfo[0].sampler = sampler_Testing;

		imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[1].imageView = *framebuffersManager->bloomImages["VERTICAL_BLOOM"]->getVkImageViewHandle();
		imageInfo[1].sampler = sampler_Testing;

		horizontalBlur_Descriptorsets[i].updateDescriptorset(bufferInfos, imageInfo);



		imageInfo.resize(1);
		imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[0].imageView = *framebuffersManager->bloomImages["HORIZONTAL_BLOOM"]->getVkImageViewHandle();
		imageInfo[0].sampler = sampler_Testing;


		finalOutPut_Descriptorsets[i].updateDescriptorset(bufferInfos, imageInfo);

	}



	for (int i = 0; i < enviromentData_Descriptorsets.size(); i++) {

		std::vector<VkDescriptorBufferInfo> bufferInfos;


		std::vector<VkDescriptorImageInfo> imageInfo;
		imageInfo.resize(5);

		imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[0].imageView = *framebuffersManager->depth_bufferImages["DEPTH_SQUARED"]->getVkImageViewHandle();
		imageInfo[0].sampler = sampler_streatch;

		imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[1].imageView = irradianceMap->getVkViewHandle();
		imageInfo[1].sampler = sampler_Testing;

		imageInfo[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[2].imageView = envMAp->getVkViewHandle();
		imageInfo[2].sampler = sampler_Testing;

		imageInfo[3].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[3].imageView = *brdfLut->getVkImageViewHandle();
		imageInfo[3].sampler = sampler_Testing;

		imageInfo[4].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		imageInfo[4].imageView = *framebuffersManager->g_bufferImages["DEPTH"]->getVkImageViewHandle();
		imageInfo[4].sampler = sampler_Testing;

		enviromentData_Descriptorsets[i].updateDescriptorset(bufferInfos, imageInfo);

	}

}



void Render::createRenderContexts()
{
	
	uint32_t n = swapChain.getNumberOfImages();

	//std::vector<VK_Objects::PComandBuffer> commandBuffers;

	//commandBuffers.resize(n);

	//for (uint32_t i = 0; i < n; i++) {

	//	commandBuffers[i] = graphicsPool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	//}

	/*for (unsigned int i = 0; i < commandBuffers.size(); i++) {

		recordCommandIndex(*commandBuffers[i], i);
	}*/

	RENDER::RenderContext render_context = RENDER::RenderContext(device, std::make_shared<VK_Objects::SwapChain>(swapChain));


	render_context.setNumberOfFrames(swapChain.getNumberOfImages());
	vkResetCommandPool(device.getLogicalDevice(), graphicsPool->getPoolHanndle(), 0);
	vkResetCommandPool(device.getLogicalDevice(), transferPool->getPoolHanndle(), 0);

	while (!glfwWindowShouldClose(w)) {

		float time = (float)glfwGetTime();
		TimeStep timeStep = time - lastFrameTIme;
		frameTime = timeStep.getTimeinMilli();
		lastFrameTIme = time;


		glfwPollEvents();
		std::stringstream ss;
		ss << "Kalm" << " " << " [" << frameTime << " MILLISEC]";

		glfwSetWindowTitle(window, ss.str().c_str());
		scriptManager.update(frameTime);

		{

			vkWaitForFences(device.getLogicalDevice(), 1, &render_context.frames[render_context.currentFrameIndex]->getFrameCountControllFence(), VK_FALSE, UINT64_MAX);
			uint32_t imageIndex;

			VkResult result = vkAcquireNextImageKHR(device.getLogicalDevice(), render_context.swapChain->getSwapChainHandle(), UINT64_MAX, render_context.frames[render_context.currentFrameIndex]->getImageAvaibleSemaphore(), VK_NULL_HANDLE, &imageIndex);
			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				recreateSwapChain();
				return;

			}
			else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
				throw std::runtime_error("failed to acquire swap chain image!");
			}
			vkResetFences(device.getLogicalDevice(), 1, &render_context.frames[render_context.currentFrameIndex]->getFrameCountControllFence());
			

			//Perform Generic Culling Operation
			perfomCulling();
			//RECORD COMMAN FOR FRAME
			recordCommandIndex(render_context.frames[render_context.currentFrameIndex]->getCommandBuffer(), render_context.currentFrameIndex);
			//swapChain->update();
			updateSceneGraph();
			updateUniforms(render_context.currentFrameIndex); 


			VkSubmitInfo submitInfo = {};
			renderUI(imageIndex);

			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { render_context.frames[render_context.currentFrameIndex]->getImageAvaibleSemaphore() };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			VkCommandBuffer cmd = render_context.frames[render_context.currentFrameIndex]->getCommandBufferHandler();

			VkCommandBuffer cmds[2] = {cmd,imGuiCmds[imageIndex] };

			submitInfo.commandBufferCount = 2;
			submitInfo.pCommandBuffers = &cmds[0];

			VkSemaphore signalSemaphores[] = { render_context.frames[render_context.currentFrameIndex]->getRenderFinishedSemaphore() };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;


			VkResult r = vkQueueSubmit(device.getQueueHandle(VK_Objects::QUEUE_TYPE::GRAPHICS), 1, &submitInfo, render_context.frames[render_context.currentFrameIndex]->getFrameCountControllFence());

			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapChains[] = { render_context.swapChain->getSwapChainHandle() };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &imageIndex;

			result = vkQueuePresentKHR(device.getQueueHandle(VK_Objects::QUEUE_TYPE::PRESENT), &presentInfo);

			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
				
				recreateSwapChain();

			}
			else if (result != VK_SUCCESS) {
				throw std::runtime_error("failed to present swap chain image!");
			}
			render_context.currentFrameIndex = (render_context.currentFrameIndex + 1) % 2;
			if(DEBUG_)
			getQueryPoolResults();

		}
		vkDeviceWaitIdle(device.getLogicalDevice());

	} 

} 

void Render::recordCommandIndex(VK_Objects::CommandBuffer& command, uint32_t i)
{
	//Record commands
	std::array<VkClearValue, 3> clearValues = {};
	clearValues[0].depthStencil = { 1.f };
	clearValues[1].color = { 1.0,1.0,1.0,.0 };
	clearValues[2].color = { 1.0,1.0 };


	renderpass->passes["G_BUFFER"]->clearValues.push_back(clearValues[1]);
	renderpass->passes["G_BUFFER"]->clearValues.push_back(clearValues[2]);
	renderpass->passes["G_BUFFER"]->clearValues.push_back(clearValues[2]);
	renderpass->passes["G_BUFFER"]->clearValues.push_back(clearValues[2]);
	renderpass->passes["G_BUFFER"]->clearValues.push_back(clearValues[0]);


	renderpass->passes["DEFERRED_LIGHTING"]->clearValues.push_back(clearValues[1]);
	renderpass->passes["DEFERRED_LIGHTING"]->clearValues.push_back(clearValues[1]);

	renderpass->passes["SWAPCHAIN_RENDERPASS"]->clearValues.push_back(clearValues[1]);

	i;
	Vk_Functions::beginCommandBuffer(command.getCommandBufferHandle());

	if (DEBUG_) {
		vkCmdResetQueryPool(command.getCommandBufferHandle(), renderQueries.statisticPool, 0, static_cast<uint32_t>(renderQueries.statistics.size()));
		vkCmdResetQueryPool(command.getCommandBufferHandle(), renderQueries.timeStampsPool, 0, static_cast<uint32_t>(renderQueries.timeStamps.size()));
	}

	if (DEBUG_)vkCmdWriteTimestamp(command.getCommandBufferHandle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, renderQueries.timeStampsPool, 0);

	createShadowMap(command.getCommandBufferHandle(), i);
	if (DEBUG_)vkCmdWriteTimestamp(command.getCommandBufferHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, renderQueries.timeStampsPool, 1);


	renderpass->passes["G_BUFFER"]->beginRenderPass(command.getCommandBufferHandle(), framebuffersManager->framebuffers["G_BUFFER"][i]->getFramebufferHandle());

	VkExtent2D e = swapChain.getExtent();
	VkViewport viewport = {};

	viewport.width = static_cast<uint32_t>(e.width);
	viewport.height = static_cast<uint32_t>(e.height);

	viewport.maxDepth = 1.0f;

	VkRect2D rect = {};
	rect.extent.width = static_cast<uint32_t>(e.width);
	rect.extent.height = static_cast<uint32_t>(e.height);
	rect.offset = { 0,0 };

	vkCmdSetViewport(command.getCommandBufferHandle(), 0, 1, &viewport);

	vkCmdSetScissor(command.getCommandBufferHandle(), 0, 1, &rect);
	if (DEBUG_) {
		vkCmdBeginQuery(command.getCommandBufferHandle(), renderQueries.statisticPool, 0, 0);
		vkCmdWriteTimestamp(command.getCommandBufferHandle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, renderQueries.timeStampsPool, 2);
	}

	VkDescriptorSet descriptorsets[1] = { globalData_Descriptorsets[i].getDescriptorSetHandle() };
	vkCmdBindDescriptorSets(command.getCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["GBUFFER_COMPOSITION"]->getPipelineLayoutHandle()->getHandle(), 0, 1, descriptorsets, 0, NULL);

	vkCmdBindPipeline(command.getCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["GBUFFER_COMPOSITION"]->getPipelineHandle());

	std::string currentTag = meshes[0]->getMaterialTag();

	for (int j = 0; j < meshes.size(); j++) {

		if (currentTag != meshes[j]->getMaterialTag() || j == 0) {
			currentTag = meshes[j]->getMaterialTag();

		}

		uint32_t dynamicOffset = j * static_cast<uint32_t>(dynamicAlignment);

		vkCmdBindDescriptorSets(command.getCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["GBUFFER_COMPOSITION"]->getPipelineLayoutHandle()->getHandle(), 2, 1, &modelMatrix_Descriptorsets[i].getDescriptorSetHandle(), 1, &dynamicOffset);
		vkCmdPushConstants(command.getCommandBufferHandle(), pipelineManager["GBUFFER_COMPOSITION"]->getPipelineLayoutHandle()->getHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Engine::Material_adjustments), &meshes[j]->getMaterialSettings());

		if(meshes[j]->isAlive() == true)
		meshes[j]->draw(command.getCommandBufferHandle(), pipelineManager, materialManager, i);

	}
	if (DEBUG_) {
		vkCmdWriteTimestamp(command.getCommandBufferHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, renderQueries.timeStampsPool, 3);
		vkCmdEndQuery(command.getCommandBufferHandle(), renderQueries.statisticPool, 0);
	}
	renderpass->passes["G_BUFFER"]->endRenderPass(command.getCommandBufferHandle());

	{

		renderpass->passes["DEFERRED_LIGHTING"]->beginRenderPass(command.getCommandBufferHandle(), framebuffersManager->framebuffers["DEFERRED_LIGHTING_FRAMEBUFFER"][i]->getFramebufferHandle());

		if (DEBUG_)vkCmdWriteTimestamp(command.getCommandBufferHandle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, renderQueries.timeStampsPool, 4);

		vkCmdBindPipeline(command.getCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["DEFERRED_SHADING"]->getPipelineHandle());

		VkDescriptorSet descriptorsets[3] = { light_Descriptorsets[i].getDescriptorSetHandle(), enviromentData_Descriptorsets[i].getDescriptorSetHandle(), deferredShading_Descriptorsets[i].getDescriptorSetHandle() };

		vkCmdBindDescriptorSets(command.getCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["DEFERRED_SHADING"]->getPipelineLayoutHandle()->getHandle(), 0, static_cast<uint32_t>(3), &descriptorsets[0], 0, NULL);

		vkCmdDraw(command.getCommandBufferHandle(), 3, 1, 0, 0);

		if (DEBUG_)vkCmdWriteTimestamp(command.getCommandBufferHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, renderQueries.timeStampsPool, 5);
		renderpass->passes["DEFERRED_LIGHTING"]->endRenderPass(command.getCommandBufferHandle());

	}

	if (DEBUG_) vkCmdWriteTimestamp(command.getCommandBufferHandle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, renderQueries.timeStampsPool, 6);

	createBloom(command.getCommandBufferHandle(), i);
	if (DEBUG_)vkCmdWriteTimestamp(command.getCommandBufferHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, renderQueries.timeStampsPool, 7);

	{
		VkViewport viewport = {};

		viewport.width = static_cast<uint32_t>(e.width);
		viewport.height = static_cast<uint32_t>(e.height);

		viewport.maxDepth = 1.0f;

		VkRect2D rect = {};
		rect.extent.width = static_cast<uint32_t>(e.width);
		rect.extent.height = static_cast<uint32_t>(e.height);
		rect.offset = { 0,0 };

		vkCmdSetViewport(command.getCommandBufferHandle(), 0, 1, &viewport);

		vkCmdSetScissor(command.getCommandBufferHandle(), 0, 1, &rect);

		if (DEBUG_)vkCmdWriteTimestamp(command.getCommandBufferHandle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, renderQueries.timeStampsPool, 8);


		renderpass->passes["SWAPCHAIN_RENDERPASS"]->beginRenderPass(command.getCommandBufferHandle(), framebuffersManager->framebuffers["SWAPCHAIN_FRAMEBUFFER"][i]->getFramebufferHandle());

		vkCmdBindPipeline(command.getCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["SWAPCHAIN_PIPELINE"]->getPipelineHandle());

		VkDescriptorSet descriptorsets[1] = { finalOutPut_Descriptorsets[i].getDescriptorSetHandle() };

		vkCmdBindDescriptorSets(command.getCommandBufferHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["SWAPCHAIN_PIPELINE"]->getPipelineLayoutHandle()->getHandle(), 0, static_cast<uint32_t>(1), &descriptorsets[0], 0, NULL);

		vkCmdDraw(command.getCommandBufferHandle(), 3, 1, 0, 0);

		renderpass->passes["SWAPCHAIN_RENDERPASS"]->endRenderPass(command.getCommandBufferHandle());

		if (DEBUG_)vkCmdWriteTimestamp(command.getCommandBufferHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, renderQueries.timeStampsPool, 9);

	}

	Vk_Functions::endCommandBuffer(command.getCommandBufferHandle());
}

void Render::createShadowMap(VkCommandBuffer& commandBuffer, uint32_t i)
{
	VkExtent2D e = swapChain.getExtent();


	//Vk_Functions::beginCommandBuffer(commandBuffer);


	std::array<VkClearValue, 2> clearValues = {};
	clearValues[1].depthStencil = { 1.f };
	clearValues[0].color = { .0,1.0,.0,.0 };


	renderpass->passes["SHADOW_MAP"]->clearValues.push_back(clearValues[0]);
	renderpass->passes["SHADOW_MAP"]->clearValues.push_back(clearValues[1]);

	renderpass->passes["SHADOW_MAP"]->beginRenderPass(commandBuffer, framebuffersManager->framebuffers["SHADOW_MAP"][i]->getFramebufferHandle());

	VkViewport viewport = {};

	viewport.width = static_cast<uint32_t>(2048);
	viewport.height = static_cast<uint32_t>(2048);

	viewport.maxDepth = 1.0f;

	VkRect2D rect = {};
	rect.extent.width = static_cast<uint32_t>(2048);
	rect.extent.height = static_cast<uint32_t>(2048);
	rect.offset = { 0,0 };

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	vkCmdSetScissor(commandBuffer, 0, 1, &rect);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["SHADOW_MAP"]->getPipelineHandle());
	
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["SHADOW_MAP"]->getPipelineLayoutHandle()->getHandle(), 0, 1, &lightProjection_Descriptorset[i].getDescriptorSetHandle(), 0, NULL);

	for (int j = 0; j < meshes.size(); j++) {

		uint32_t dynamicOffset = j * static_cast<uint32_t>(dynamicAlignment);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["SHADOW_MAP"]->getPipelineLayoutHandle()->getHandle(), 1, 1, &modelMatrix_Descriptorsets[i].getDescriptorSetHandle(), 1, &dynamicOffset);

		meshes[j]->draw(commandBuffer);
	}

	renderpass->passes["SHADOW_MAP"]->endRenderPass(commandBuffer);

}

void Render::createBloom(VkCommandBuffer& commandBuffer, uint32_t i)
{

	VkExtent2D e = swapChain.getExtent();
	e.width /= 2.0;
	e.height /= 2.0;
	VkViewport viewport = {};

	viewport.width = static_cast<uint32_t>(e.width);
	viewport.height = static_cast<uint32_t>(e.height);

	viewport.maxDepth = 1.0f;

	VkRect2D rect = {};
	rect.extent.width = static_cast<uint32_t>(e.width);
	rect.extent.height = static_cast<uint32_t>(e.height);
	rect.offset = { 0,0 };

	VkClearValue clearValues[2];
	clearValues[0].color = { 1.0f,.0f, 1.0f, 1.f };
	clearValues[1].color = { 1.0f,.0f, 1.0f, 1.f };

	renderpass->passes["VERTICAL_BLUR"]->clearValues.push_back(clearValues[1]);
	renderpass->passes["HORIZONTAL_BLUR"]->clearValues.push_back(clearValues[0]);
	renderpass->passes["HORIZONTAL_BLUR"]->clearValues.push_back(clearValues[1]);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &rect);

	renderpass->passes["VERTICAL_BLUR"]->beginRenderPass(commandBuffer, framebuffersManager->framebuffers["VERTICAL_FRAMEBUFFER"][i]->getFramebufferHandle());

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["VERTICAL_BLUR_PIPELINE"]->getPipelineHandle());

	VkDescriptorSet descriptorsets[1] = { verticalBlur_Descriptorsets[i].getDescriptorSetHandle() };

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["VERTICAL_BLUR_PIPELINE"]->getPipelineLayoutHandle()->getHandle(), 0, static_cast<uint32_t>(1), &descriptorsets[0], 0, NULL);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	renderpass->passes["VERTICAL_BLUR"]->endRenderPass(commandBuffer);

	{
		// ------ Second Pass Horizontal Blue
		renderpass->passes["HORIZONTAL_BLUR"]->beginRenderPass(commandBuffer, framebuffersManager->framebuffers["HORIZONTAL_FRAMEBUFFER"][i]->getFramebufferHandle());

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["HORIZONTAL_BLUR_PIPELINE"]->getPipelineHandle());

		VkDescriptorSet descriptorsets[1] = { horizontalBlur_Descriptorsets[i].getDescriptorSetHandle() };

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager["HORIZONTAL_BLUR_PIPELINE"]->getPipelineLayoutHandle()->getHandle(), 0, static_cast<uint32_t>(1), &descriptorsets[0], 0, NULL);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		renderpass->passes["HORIZONTAL_BLUR"]->endRenderPass(commandBuffer);
	}
}

void Render::createPipeline()
{

	//Game Descriptor Manager.
	poolManager = std::make_shared<VK_Objects::DescriptorPoolManager>(&device);

	//Gbuffer Composition Pipeline

	VK_Objects::ShaderResource viewProjection{};
	viewProjection.binding = static_cast<uint32_t>(0);
	viewProjection.size = sizeof(VP);
	viewProjection.stages = VK_SHADER_STAGE_VERTEX_BIT;
	viewProjection.type = VK_Objects::ShaderResourceType::UNIFORM_BUFFER;

	VK_Objects::ShaderResource diffuseTextureResource{};
	diffuseTextureResource.binding = static_cast<uint32_t>(0);
	diffuseTextureResource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	diffuseTextureResource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

	VK_Objects::ShaderResource emissionMapResource{};
	emissionMapResource.binding = static_cast<uint32_t>(1);
	emissionMapResource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	emissionMapResource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

	VK_Objects::ShaderResource roughnessMapResource{};
	roughnessMapResource.binding = static_cast<uint32_t>(2);
	roughnessMapResource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	roughnessMapResource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;


	VK_Objects::ShaderResource metallicMapResource{};
	metallicMapResource.binding = static_cast<uint32_t>(3);
	metallicMapResource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	metallicMapResource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

	VK_Objects::ShaderResource normalMapResource{};
	normalMapResource.binding = static_cast<uint32_t>(4);
	normalMapResource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	normalMapResource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;


	VK_Objects::ShaderResource modelMatrix{};
	modelMatrix.binding = static_cast<uint32_t>(0);
	modelMatrix.size = sizeof(ModelMatrix);
	modelMatrix.stages = VK_SHADER_STAGE_VERTEX_BIT;
	modelMatrix.type = VK_Objects::ShaderResourceType::UNIFORM_DYNAMIC;



	//Create shader resource and Allocate Descriptorsets 
	std::vector<VK_Objects::ShaderResource> resources = { viewProjection };

	std::shared_ptr<VK_Objects::DescriptorSetLayout> descLayout = std::make_shared<VK_Objects::DescriptorSetLayout>(&device, resources);

	std::vector<VK_Objects::ShaderResource> resourceMaterial = { diffuseTextureResource,emissionMapResource, metallicMapResource, roughnessMapResource, normalMapResource };

	std::shared_ptr<VK_Objects::DescriptorSetLayout> descLayoutMaterial = std::make_shared<VK_Objects::DescriptorSetLayout>(&device, resourceMaterial);


	std::vector<VK_Objects::ShaderResource> modelMatrixResource = { modelMatrix };

	std::shared_ptr<VK_Objects::DescriptorSetLayout> modelMatrixDescLayout = std::make_shared<VK_Objects::DescriptorSetLayout>(&device, modelMatrixResource);

	for (unsigned int i = 0; i < swapChain.getNumberOfImages(); i++) {

		//Allocate descriptorset for descLayout and store it inside sets variable.
		globalData_Descriptorsets.push_back(poolManager->allocateDescriptor(descLayout));
		modelMatrix_Descriptorsets.push_back(poolManager->allocateDescriptor(modelMatrixDescLayout));

	}

	std::vector<std::shared_ptr<VK_Objects::DescriptorSetLayout>> descriptors;
	descriptors.push_back(std::move(descLayout));
	descriptors.push_back(std::move(descLayoutMaterial));
	descriptors.push_back(std::move(modelMatrixDescLayout));


	//Create Pipeline
	std::unique_ptr<VK_Objects::Shader> vert = std::make_unique< VK_Objects::Shader>(device, VK_Objects::SHADER_TYPE::VERTEX_SHADER, Utils::readFile("Shaders\\gBufferComposition\\vert.spv"));
	std::unique_ptr<VK_Objects::Shader> frag = std::make_unique< VK_Objects::Shader>(device, VK_Objects::SHADER_TYPE::FRAGMENT_SHADER, Utils::readFile("Shaders\\gBufferComposition\\frag.spv"));

	struct {
		glm::vec2 texOffset;
		float rough_multiplier;
	}pushData;

	std::vector<VkPushConstantRange> pushConstantsData;
	pushConstantsData.resize(1);
	pushConstantsData[0].offset = 0;
	pushConstantsData[0].size = sizeof(Engine::Material_adjustments);
	pushConstantsData[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;



	std::unique_ptr<VK_Objects::PipelineLayout> layout = std::make_unique<VK_Objects::PipelineLayout>(device, std::move(descriptors), pushConstantsData);

	VK_Objects::PipelineProperties pipelineInfo{};

	std::vector<VK_Objects::ATRIBUTES> atributes = { VK_Objects::ATRIBUTES::VEC3,VK_Objects::ATRIBUTES::VEC3,VK_Objects::ATRIBUTES::VEC2 };

	std::vector<std::vector<VK_Objects::ATRIBUTES>>att{ atributes };

	pipelineInfo.atributes = att;
	pipelineInfo.colorAttachmentsCount = 4;
	pipelineInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
	pipelineInfo.dephTest = 1;
	pipelineInfo.depthBias = 0;
	pipelineInfo.rdpass = &renderpass->passes["G_BUFFER"]->vk_renderpass;
	pipelineInfo.frontFaceClock = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pipelineInfo.vertexOffsets = { 0 };
	pipelineInfo.subpass = 0;

	std::unique_ptr<VK_Objects::Pipeline> gBufferCompositionPipeline = std::make_unique<VK_Objects::Pipeline>(device, std::move(layout), std::move(vert), std::move(frag), pipelineInfo);
	gBufferCompositionPipeline->id = "GBUFFER_COMPOSITION";

	pipelineManager["GBUFFER_COMPOSITION"] = std::move(gBufferCompositionPipeline);

	//Deferred Lighting Pipeline
	{

		VK_Objects::ShaderResource albedoResourceInput{};
		albedoResourceInput.binding = static_cast<uint32_t>(0);
		albedoResourceInput.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		albedoResourceInput.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

		VK_Objects::ShaderResource metallicRoughnessInput{};
		metallicRoughnessInput.binding = static_cast<uint32_t>(1);
		metallicRoughnessInput.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		metallicRoughnessInput.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

		VK_Objects::ShaderResource normalsInput{};
		normalsInput.binding = static_cast<uint32_t>(2);
		normalsInput.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		normalsInput.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

		VK_Objects::ShaderResource emissionMapResource{};
		emissionMapResource.binding = static_cast<uint32_t>(3);
		emissionMapResource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		emissionMapResource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;




		//Create shader resource and Allocate Descriptorsets 
		std::vector<VK_Objects::ShaderResource> resourcess = { albedoResourceInput , metallicRoughnessInput , normalsInput,emissionMapResource };

		std::shared_ptr<VK_Objects::DescriptorSetLayout> descriptorsetForMaterial = std::make_shared<VK_Objects::DescriptorSetLayout>(&device, resourcess);


		VK_Objects::ShaderResource descriptorsetForShadowMap{};
		descriptorsetForShadowMap.binding = static_cast<uint32_t>(0);
		descriptorsetForShadowMap.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorsetForShadowMap.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;


		VK_Objects::ShaderResource descriptorsetForIrradianceMap{};
		descriptorsetForIrradianceMap.binding = static_cast<uint32_t>(1);
		descriptorsetForIrradianceMap.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorsetForIrradianceMap.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;


		VK_Objects::ShaderResource descriptorsetForEnvMap{};
		descriptorsetForEnvMap.binding = static_cast<uint32_t>(2);
		descriptorsetForEnvMap.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorsetForEnvMap.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;


		VK_Objects::ShaderResource descriptorsetForBRDFLut{};
		descriptorsetForBRDFLut.binding = static_cast<uint32_t>(3);
		descriptorsetForBRDFLut.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorsetForBRDFLut.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

		VK_Objects::ShaderResource descriptorsetForDepthMap{};
		descriptorsetForDepthMap.binding = static_cast<uint32_t>(4);
		descriptorsetForDepthMap.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorsetForDepthMap.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

		std::vector<VK_Objects::ShaderResource> resourcessSlot1 = { descriptorsetForShadowMap , descriptorsetForIrradianceMap , descriptorsetForEnvMap,descriptorsetForBRDFLut,descriptorsetForDepthMap };

		std::shared_ptr<VK_Objects::DescriptorSetLayout> descriptorForEnviroment = std::make_shared<VK_Objects::DescriptorSetLayout>(&device, resourcessSlot1);


		for (unsigned int i = 0; i < swapChain.getNumberOfImages(); i++) {
			//Allocate descriptorset for descLayout and store it inside sets variable.
			enviromentData_Descriptorsets.push_back((poolManager->allocateDescriptor(descriptorForEnviroment)));
			deferredShading_Descriptorsets.push_back((poolManager->allocateDescriptor(descriptorsetForMaterial)));

		}

		std::vector<std::shared_ptr<VK_Objects::DescriptorSetLayout>> descriptors;


		VK_Objects::ShaderResource lightResource{};

		lightResource.binding = static_cast<uint32_t>(0);
		lightResource.size = sizeof(LightUniform);
		lightResource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		lightResource.type = VK_Objects::ShaderResourceType::UNIFORM_BUFFER;

		std::vector<VK_Objects::ShaderResource> lightResourceData = { lightResource };

		std::shared_ptr<VK_Objects::DescriptorSetLayout> lightDescLayout = std::make_shared<VK_Objects::DescriptorSetLayout>(&device, lightResourceData);


		for (unsigned int i = 0; i < swapChain.getNumberOfImages(); i++) {

			light_Descriptorsets.push_back((poolManager->allocateDescriptor(lightDescLayout)));

		}


		descriptors.push_back(std::move(lightDescLayout));
		descriptors.push_back(std::move(descriptorForEnviroment));
		descriptors.push_back(std::move(descriptorsetForMaterial));

		//Create Pipeline
		std::unique_ptr<VK_Objects::Shader> vert = std::make_unique< VK_Objects::Shader>(device, VK_Objects::SHADER_TYPE::VERTEX_SHADER, Utils::readFile("Shaders\\deferredLighting\\vert.spv"));
		std::unique_ptr<VK_Objects::Shader> frag = std::make_unique< VK_Objects::Shader>(device, VK_Objects::SHADER_TYPE::FRAGMENT_SHADER, Utils::readFile("Shaders\\deferredLighting\\frag.spv"));

		std::vector<VkPushConstantRange> pushConstants;

		std::unique_ptr<VK_Objects::PipelineLayout> layout = std::make_unique<VK_Objects::PipelineLayout>(device, std::move(descriptors), pushConstants);

		VK_Objects::PipelineProperties pipelineInfo{};

		std::vector<VK_Objects::ATRIBUTES> atributes{ };

		std::vector<std::vector<VK_Objects::ATRIBUTES>>att{ atributes };

		pipelineInfo.atributes = att;
		pipelineInfo.colorAttachmentsCount = 2;
		pipelineInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
		pipelineInfo.dephTest = 0;
		pipelineInfo.depthBias = 0;
		pipelineInfo.rdpass = &renderpass->passes["DEFERRED_LIGHTING"]->vk_renderpass;
		pipelineInfo.frontFaceClock = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipelineInfo.vertexOffsets = { 0 };
		pipelineInfo.subpass = 0;

		std::unique_ptr<VK_Objects::Pipeline> deferredLightingPipeline = std::make_unique<VK_Objects::Pipeline>(device, std::move(layout), std::move(vert), std::move(frag), pipelineInfo);
		deferredLightingPipeline->id = "DEFERRED_SHADING";

		pipelineManager["DEFERRED_SHADING"] = std::move(deferredLightingPipeline);

	}


	//Shadow Map pipeline
	{


		//Create shader resource and Allocate Descriptorsets 
		std::vector<VK_Objects::ShaderResource> resourcess = { modelMatrix };

		std::shared_ptr<VK_Objects::DescriptorSetLayout> modelMatrixDescLayout = std::make_shared<VK_Objects::DescriptorSetLayout>(&device, resourcess);

		std::vector<std::shared_ptr<VK_Objects::DescriptorSetLayout>> descriptors;


		VK_Objects::ShaderResource lightResource{};

		lightResource.binding = static_cast<uint32_t>(0);
		lightResource.size = sizeof(glm::mat4);
		lightResource.stages = VK_SHADER_STAGE_VERTEX_BIT;
		lightResource.type = VK_Objects::ShaderResourceType::UNIFORM_BUFFER;

		std::vector<VK_Objects::ShaderResource> lightResourceData = { lightResource };

		std::shared_ptr<VK_Objects::DescriptorSetLayout> lightDescLayout = std::make_shared<VK_Objects::DescriptorSetLayout>(&device, lightResourceData);

		for (unsigned int i = 0; i < swapChain.getNumberOfImages(); i++) {

			lightProjection_Descriptorset.push_back((poolManager->allocateDescriptor(lightDescLayout)));

		}




		descriptors.push_back(std::move(lightDescLayout));
		descriptors.push_back(std::move(modelMatrixDescLayout));

		//Create Pipeline
		std::unique_ptr<VK_Objects::Shader> vert = std::make_unique< VK_Objects::Shader>(device, VK_Objects::SHADER_TYPE::VERTEX_SHADER, Utils::readFile("Shaders\\shadowMap\\vert.spv"));
		std::unique_ptr<VK_Objects::Shader> frag = std::make_unique< VK_Objects::Shader>(device, VK_Objects::SHADER_TYPE::FRAGMENT_SHADER, Utils::readFile("Shaders\\shadowMap\\frag.spv"));

		std::vector<VkPushConstantRange> pushConstants;

		std::unique_ptr<VK_Objects::PipelineLayout> layout = std::make_unique<VK_Objects::PipelineLayout>(device, std::move(descriptors), pushConstants);

		VK_Objects::PipelineProperties pipelineInfo{};

		std::vector<VK_Objects::ATRIBUTES> atributes = { VK_Objects::ATRIBUTES::VEC3,VK_Objects::ATRIBUTES::VEC3,VK_Objects::ATRIBUTES::VEC2 };

		std::vector<std::vector<VK_Objects::ATRIBUTES>>att{ atributes };

		pipelineInfo.atributes = att;
		pipelineInfo.colorAttachmentsCount = 1;
		pipelineInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
		pipelineInfo.dephTest = 1;
		pipelineInfo.depthBias = 1;
		pipelineInfo.rdpass = &renderpass->passes["SHADOW_MAP"]->vk_renderpass;
		pipelineInfo.frontFaceClock = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipelineInfo.vertexOffsets = { 0 };
		pipelineInfo.subpass = 0;

		std::unique_ptr<VK_Objects::Pipeline> shadowMapPipeline = std::make_unique<VK_Objects::Pipeline>(device, std::move(layout), std::move(vert), std::move(frag), pipelineInfo);
		shadowMapPipeline->id = "SHADOW_MAP";

		pipelineManager["SHADOW_MAP"] = std::move(shadowMapPipeline);

	}

	{

		//vertical blur  Pipeline
		
		VK_Objects::ShaderResource verticalBlurImageRessource{};
		verticalBlurImageRessource.binding = static_cast<uint32_t>(0);
		verticalBlurImageRessource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		verticalBlurImageRessource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

		//Create shader resource and Allocate Descriptorsets 
		std::vector<VK_Objects::ShaderResource> resources = { verticalBlurImageRessource };

		std::shared_ptr<VK_Objects::DescriptorSetLayout> descLayout = std::make_shared<VK_Objects::DescriptorSetLayout>(&device, resources);

		for (unsigned int i = 0; i < swapChain.getNumberOfImages(); i++) {

			//Allocate descriptorset for descLayout and store it inside sets variable.
			verticalBlur_Descriptorsets.push_back(poolManager->allocateDescriptor(descLayout));

		}

		std::vector<std::shared_ptr<VK_Objects::DescriptorSetLayout>> descriptors;
		descriptors.push_back(std::move(descLayout));


		//Create Pipeline
		std::unique_ptr<VK_Objects::Shader> vert = std::make_unique< VK_Objects::Shader>(device, VK_Objects::SHADER_TYPE::VERTEX_SHADER, Utils::readFile("Shaders\\bloom\\vertical\\vert.spv"));
		std::unique_ptr<VK_Objects::Shader> frag = std::make_unique< VK_Objects::Shader>(device, VK_Objects::SHADER_TYPE::FRAGMENT_SHADER, Utils::readFile("Shaders\\bloom\\vertical\\frag.spv"));

		std::vector<VkPushConstantRange> pushConstants;

		std::unique_ptr<VK_Objects::PipelineLayout> layout = std::make_unique<VK_Objects::PipelineLayout>(device, std::move(descriptors), pushConstants);

		VK_Objects::PipelineProperties pipelineInfo{};

		std::vector<VK_Objects::ATRIBUTES> atributes = { };

		std::vector<std::vector<VK_Objects::ATRIBUTES>>att{ atributes };

		pipelineInfo.atributes = att;
		pipelineInfo.colorAttachmentsCount = 1;
		pipelineInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
		pipelineInfo.dephTest = 0;
		pipelineInfo.depthBias = 0;
		pipelineInfo.rdpass = &renderpass->passes["VERTICAL_BLUR"]->vk_renderpass;
		pipelineInfo.frontFaceClock = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipelineInfo.vertexOffsets = { 0 };
		pipelineInfo.subpass = 0;

		std::unique_ptr<VK_Objects::Pipeline> verticalBlurPipeline = std::make_unique<VK_Objects::Pipeline>(device, std::move(layout), std::move(vert), std::move(frag), pipelineInfo);
		verticalBlurPipeline->id = "VERTICAL_BLUR_PIPELINE";

		pipelineManager["VERTICAL_BLUR_PIPELINE"] = std::move(verticalBlurPipeline);

	}
	{

		VK_Objects::ShaderResource HorizontalBlurImageRessource{};
		HorizontalBlurImageRessource.binding = static_cast<uint32_t>(1);
		HorizontalBlurImageRessource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		HorizontalBlurImageRessource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;
		
		VK_Objects::ShaderResource ImageRessource{};
		ImageRessource.binding = static_cast<uint32_t>(0);
		ImageRessource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		ImageRessource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

		//Create shader resource and Allocate Descriptorsets 
		std::vector<VK_Objects::ShaderResource> resources = { HorizontalBlurImageRessource,ImageRessource };

		std::shared_ptr<VK_Objects::DescriptorSetLayout> descLayout = std::make_shared<VK_Objects::DescriptorSetLayout>(&device, resources);

		for (unsigned int i = 0; i < swapChain.getNumberOfImages(); i++) {

			//Allocate descriptorset for descLayout and store it inside sets variable.
			horizontalBlur_Descriptorsets.push_back(poolManager->allocateDescriptor(descLayout));

		}

		std::vector<std::shared_ptr<VK_Objects::DescriptorSetLayout>> descriptors;
		descriptors.push_back(std::move(descLayout));


		//Create Pipeline
		std::unique_ptr<VK_Objects::Shader> vert = std::make_unique< VK_Objects::Shader>(device, VK_Objects::SHADER_TYPE::VERTEX_SHADER, Utils::readFile("Shaders\\bloom\\horizontal\\vert.spv"));
		std::unique_ptr<VK_Objects::Shader> frag = std::make_unique< VK_Objects::Shader>(device, VK_Objects::SHADER_TYPE::FRAGMENT_SHADER, Utils::readFile("Shaders\\bloom\\horizontal\\frag.spv"));

		std::vector<VkPushConstantRange> pushConstants;

		std::unique_ptr<VK_Objects::PipelineLayout> layout = std::make_unique<VK_Objects::PipelineLayout>(device, std::move(descriptors), pushConstants);

		VK_Objects::PipelineProperties pipelineInfo{};

		std::vector<VK_Objects::ATRIBUTES> atributes = { };

		std::vector<std::vector<VK_Objects::ATRIBUTES>>att{ atributes };

		pipelineInfo.atributes = att;
		pipelineInfo.colorAttachmentsCount = 1;
		pipelineInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
		pipelineInfo.dephTest = 0;
		pipelineInfo.depthBias = 0;
		pipelineInfo.rdpass = &renderpass->passes["HORIZONTAL_BLUR"]->vk_renderpass;
		pipelineInfo.frontFaceClock = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipelineInfo.vertexOffsets = { 0 };
		pipelineInfo.subpass = 0;

		std::unique_ptr<VK_Objects::Pipeline> horizontalBlurPipeline = std::make_unique<VK_Objects::Pipeline>(device, std::move(layout), std::move(vert), std::move(frag), pipelineInfo);
		horizontalBlurPipeline->id = "HORIZONTAL_BLUR";

		pipelineManager["HORIZONTAL_BLUR_PIPELINE"] = std::move(horizontalBlurPipeline);
	}


	{
		//SwapChain Presentation pipeline
		VK_Objects::ShaderResource finalOutputResource{};
		finalOutputResource.binding = static_cast<uint32_t>(0);
		finalOutputResource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		finalOutputResource.type = VK_Objects::ShaderResourceType::IMAGE_SAMPLER;

		//Create shader resource and Allocate Descriptorsets 
		std::vector<VK_Objects::ShaderResource> resources = { finalOutputResource };

		std::shared_ptr<VK_Objects::DescriptorSetLayout> descLayout = std::make_shared<VK_Objects::DescriptorSetLayout>(&device, resources);

		for (unsigned int i = 0; i < swapChain.getNumberOfImages(); i++) {

			//Allocate descriptorset for descLayout and store it inside sets variable.
			finalOutPut_Descriptorsets.push_back(poolManager->allocateDescriptor(descLayout));

		}

		std::vector<std::shared_ptr<VK_Objects::DescriptorSetLayout>> descriptors;
		descriptors.push_back(std::move(descLayout));


		//Create Pipeline
		std::unique_ptr<VK_Objects::Shader> vert = std::make_unique< VK_Objects::Shader>(device, VK_Objects::SHADER_TYPE::VERTEX_SHADER, Utils::readFile("Shaders\\presentation\\vert.spv"));
		std::unique_ptr<VK_Objects::Shader> frag = std::make_unique< VK_Objects::Shader>(device, VK_Objects::SHADER_TYPE::FRAGMENT_SHADER, Utils::readFile("Shaders\\presentation\\frag.spv"));

		std::vector<VkPushConstantRange> pushConstants;

		std::unique_ptr<VK_Objects::PipelineLayout> layout = std::make_unique<VK_Objects::PipelineLayout>(device, std::move(descriptors), pushConstants);

		VK_Objects::PipelineProperties pipelineInfo{};

		std::vector<VK_Objects::ATRIBUTES> atributes = { };

		std::vector<std::vector<VK_Objects::ATRIBUTES>>att{ atributes };

		pipelineInfo.atributes = att;
		pipelineInfo.colorAttachmentsCount = 1;
		pipelineInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
		pipelineInfo.dephTest = 0;
		pipelineInfo.depthBias = 0;
		pipelineInfo.rdpass = &renderpass->passes["SWAPCHAIN_RENDERPASS"]->vk_renderpass;
		pipelineInfo.frontFaceClock = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipelineInfo.vertexOffsets = { 0 };
		pipelineInfo.subpass = 0;
		std::unique_ptr<VK_Objects::Pipeline> SWAPCHAIN_PIPELINE = std::make_unique<VK_Objects::Pipeline>(device, std::move(layout), std::move(vert), std::move(frag), pipelineInfo);
		SWAPCHAIN_PIPELINE->id = "SWAPCHAIN_PIPELINE";

		pipelineManager["SWAPCHAIN_PIPELINE"] = std::move(SWAPCHAIN_PIPELINE);

	}
}

void Render::createCommandPools()
{
	graphicsPool = std::make_unique<VK_Objects::CommandPool>(device, VK_Objects::POOL_TYPE::GRAPHICS, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	dynamicPool = std::make_unique<VK_Objects::CommandPool>(device, VK_Objects::POOL_TYPE::GRAPHICS, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	transferPool = std::make_unique<VK_Objects::CommandPool>(device, VK_Objects::POOL_TYPE::TRANSFER, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
}


void Render::createDynamicUniformBuffers()
{
	modelBuffers.resize(swapChain.getNumberOfImages());


	size_t minUboAlignment = device.getMinimumBufferAligment();
	dynamicAlignment = sizeof(glm::mat4);

	if (minUboAlignment > 0) {
		dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}

	for (int i = 0; i < modelBuffers.size(); i++)
		modelBuffers[i] = std::make_shared<VK_Objects::Buffer>(&device, static_cast<uint32_t>(meshes.size()) * dynamicAlignment, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);


	modelBuffersSize = static_cast<uint32_t>(meshes.size()) * dynamicAlignment;
	modelMatrixes.model = (glm::mat4*)alignedAlloc(modelBuffersSize, dynamicAlignment);
	assert(modelMatrixes.model);

}

void Render::createMaterials()
{
	Engine::FilesPath path{};
	path.diffuseMap = "Assets\\common\\white.png";
	path.emissionMap = "Assets\\common\\black.png";
	path.index = 0;
	path.metallicMap = "Assets\\common\\black.png";
	path.name = "default_material";
	path.normalMap = "Assets\\common\\black.png";
	path.roughnessMap = "Assets\\common\\white.png";

	//Create DeafultMaterial
	materialManager["default_material"] = std::make_unique<Engine::Material>(&device, "default_material", path, poolManager, transferPool.get(), graphicsPool.get(), swapChain.getNumberOfImages());


	for (auto mesh : meshes) {
		if (mesh->getMaterialTag() == "default_material")continue;
		std::string s = mesh->getId();
		std::vector<Engine::FilesPath> files = mesh->getTextureFIles();
		for (auto mat : files) {

			if(materialManager.find(mat.name) == materialManager.end())
 				materialManager[mat.name] = std::make_unique<Engine::Material>(&device, mat.name, files[mat.index], poolManager, transferPool.get(), graphicsPool.get(), swapChain.getNumberOfImages());
		}
	}
	//materialManager["Player"] = std::make_unique<Engine::Material>(&device, "Player", path, poolManager, transferPool.get(), graphicsPool.get(), swapChain.getNumberOfImages());
	
	//materialManager["MetallicTile"] = std::make_unique<Engine::Material>(&device, "MetallicTile", path, poolManager, transferPool.get(), graphicsPool.get(), swapChain.getNumberOfImages());

}

void Render::createSamplers()
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 4;
	samplerInfo.maxLod = 10;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;


	if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, device.getAllocator(), &sampler_streatch) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	Vk_Functions::createSampler(&device, sampler_Testing);

}


void Render::createScene()
{
	Game::Scene scene;
	SceneGraph& sceneGraph = scene.sceneGraph;
	//		Light(const char* id, glm::vec3 color = glm::vec3(1.0f), glm::vec3 position = glm::vec3(0.0), float type = 0.0);

	sceneGraph.root->entity->transform->setScale(glm::vec3(0.7, 1.0, 0.7));
	//Lights
	std::shared_ptr<Engine::Entity>  sun = std::make_shared<Engine::Light>("Sun",glm::vec3(1),glm::vec3(100,100,0),0.0);

	std::shared_ptr<Engine::Entity>  pointLight1 = std::make_shared<Engine::Light>("pointLight1", glm::vec3(1), glm::vec3(10, 4, 1), 1.0);

	std::shared_ptr<Engine::Entity>  pointLight2 = std::make_shared<Engine::Light>("pointLight2", glm::vec3(1), glm::vec3(-10, 4, -1), 1.0);

	std::shared_ptr<Engine::Entity>  camera_entity = std::make_shared<Engine::Camera>("MainCamera");

	main_camera = std::dynamic_pointer_cast<Engine::Camera>(camera_entity);

	std::shared_ptr<Engine::Script> cameraController = std::make_shared<CameraController>(main_camera, "camController");

	camera_entity->attachComponent(cameraController);
	int h, w;
	glfwGetFramebufferSize(window, &w, &h);
	main_camera->setWidthHeight(w, h);

	std::dynamic_pointer_cast<Engine::Transform>(main_camera->getComponent(Engine::COMPONENT_TYPE::TRANSFORM))->setPosition(-.44, 1.351, -14.5);
	
	std::shared_ptr<Node> cameraNode = std::make_shared<Node>(camera_entity);

	std::shared_ptr<Engine::Entity> cube = std::make_shared<Engine::Entity>("plane");
	cube->transform->setScale(glm::vec3(5, 0.1, 5.));

	{

		for (int i = 0; i < 10; i++) {
			for (int j = -5; j < 5; j++) {
				std::shared_ptr<Engine::Entity> globe = std::make_shared<Engine::Entity>("MyGlobe");
				globe->transform->setPosition(1.8*i, 2*j, .0);
				globe->attachComponent(std::make_shared<Engine::Mesh>(globe, "MyGlobe", "MyGlobe", "Assets\\MyGlobe\\globe.gltf", &device, transferPool.get()));
				std::shared_ptr<Node> node6 = std::make_shared<Node>(globe);
				sceneGraph.addNode(node6);


			}
		}


	}
	cube->attachComponent(std::make_shared<Engine::Mesh>(cube, "plane", "plane", "Assets\\plane\\plane.gltf", &device, transferPool.get()));

	//std::shared_ptr<Node> node1 = std::make_shared<Node>(Floor);
	std::shared_ptr<Node> node3 = std::make_shared<Node>(sun);
	std::shared_ptr<Node> node4 = std::make_shared<Node>(pointLight1);
	std::shared_ptr<Node> node5 = std::make_shared<Node>(pointLight2);
	std::shared_ptr<Node> node7 = std::make_shared<Node>(cube);

	 
	sceneGraph.addNode(cameraNode);
	//sceneGraph.addNode(node1);
	sceneGraph.addNode(node3);
	sceneGraph.addNode(node4);
	sceneGraph.addNode(node5);
	sceneGraph.addNode(node7);

	mainSCene = std::move(scene);
	mainSCene.sceneGraph.updateSceneGraph();

}

void Render::separateSceneObjects(std::shared_ptr<Node> node)
{

	std::shared_ptr<Engine::Entity> e1 = node->entity;


		if ((e1)->getComponent(Engine::COMPONENT_TYPE::MESH)) {

			std::shared_ptr<Engine::Mesh> m = std::dynamic_pointer_cast<Engine::Mesh>((e1)->getComponent(Engine::COMPONENT_TYPE::MESH));
			meshes.push_back(m);

		}

		if ((e1)->getComponent(Engine::COMPONENT_TYPE::SCRIPT)) {

			std::shared_ptr<Engine::Script> m = std::dynamic_pointer_cast<Engine::Script>((e1)->getComponent(Engine::COMPONENT_TYPE::SCRIPT));
			scriptManager.insertScript(m);

		}

		if ((e1)->getComponent(Engine::COMPONENT_TYPE::LIGHT)) {

			std::shared_ptr<Engine::LightComponent> m = std::dynamic_pointer_cast<Engine::LightComponent>((e1)->getComponent(Engine::COMPONENT_TYPE::LIGHT));
			lightContainer.push_back(m);

		}
		std::list<std::shared_ptr<Node>>::iterator it = node->childs.begin();


		while (it != node->childs.end()) {
			separateSceneObjects(*it);
			it++;
		}

}

void Render::getEntityScripts()
{

}

void Render::createImGuiInterface()
{

	imGuiCmds.resize(swapChain.getNumberOfImages());


	for (auto& cmd : imGuiCmds)
		cmd = dynamicPool->requestCommandBufferVK(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("Engine\\font\\Roboto-Light.ttf",16.0f);

	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImColor(255, 255, 255);
	style.Colors[ImGuiCol_TitleBg] = ImColor(255, 255, 255);
	style.Colors[ImGuiCol_Border] = ImColor(0, 0, 0);
	style.Colors[ImGuiCol_Text] = ImColor(0, 0, 0);
	style.Colors[ImGuiCol_ChildBg] = ImColor(0, 0, 0);
	style.Colors[ImGuiCol_Tab] = ImColor(255, 255, 255);
	style.Colors[ImGuiCol_TabActive] = ImColor(255, 255, 255);
	style.Colors[ImGuiCol_TabHovered] = ImColor(225, 225, 225);
	style.Colors[ImGuiCol_FrameBg] = ImColor(255, 255, 255);

	style.WindowRounding = 3.0f;
	
	style.WindowBorderSize = 2.5f;
	style.FrameBorderSize = 1.5f;;
	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance.vk_Instance;
	init_info.PhysicalDevice = device.getPhysicalDevice();
	init_info.Device = device.getLogicalDevice();
	init_info.QueueFamily = device.getGraphicsQueueIndex();
	init_info.Queue = device.getQueueHandle(VK_Objects::QUEUE_TYPE::GRAPHICS);
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = poolManager->getDescriptorPoolHandle();
	init_info.Allocator = VK_NULL_HANDLE;
	init_info.MinImageCount = swapChain.getNumberOfImages();
	init_info.ImageCount = swapChain.getNumberOfImages();
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info, renderpass->passes["INTERFACE"]->vk_renderpass);

	std::unique_ptr<VK_Objects::CommandBuffer>cmd = dynamicPool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	Vk_Functions::beginCommandBuffer(cmd->getCommandBufferHandle());

	ImGui_ImplVulkan_CreateFontsTexture(cmd->getCommandBufferHandle(),*transferPool.get(),device);


	Vk_Functions::endCommandBuffer(cmd->getCommandBufferHandle());


}

void Render::setqueryPoolStatistics()
{
	VkQueryPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
	info.pipelineStatistics =
		VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
		VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
		VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
		VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
		VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
		VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;
	info.queryCount = 6;
	renderQueries.statistics.resize(6);

	if (vkCreateQueryPool(device.getLogicalDevice(), &info, device.getAllocator(), &renderQueries.statisticPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create QueryPool");

	}

	{
		VkQueryPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		info.queryType = VK_QUERY_TYPE_TIMESTAMP;
		info.queryCount = 10;
		renderQueries.timeStamps.resize(10);

		if (vkCreateQueryPool(device.getLogicalDevice(), &info, device.getAllocator(), &renderQueries.timeStampsPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create QueryPool");

		}
	}
}

void Render::getQueryPoolResults()
{
	uint32_t count = static_cast<uint32_t>(renderQueries.statistics.size());
	vkGetQueryPoolResults(
		device.getLogicalDevice(),
		renderQueries.statisticPool,
		0,
		1,
		count * sizeof(uint64_t),
		renderQueries.statistics.data(),
		sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT);


	vkGetQueryPoolResults(
		device.getLogicalDevice(),
		renderQueries.timeStampsPool,
		0,
		10,
		renderQueries.timeStamps.size() * sizeof(uint64_t),
		renderQueries.timeStamps.data(),
		sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT);


}

void Render::renderUI(uint32_t imageIndex)
{
	Vk_Functions::beginCommandBuffer(imGuiCmds[imageIndex]);

	VkExtent2D extent = swapChain.getExtent();

	VkClearValue clearValues;

	clearValues.color = { 1.0f,.0f, 1.0f, 1.f };
	renderpass->passes["INTERFACE"]->clearValues = { clearValues };

	VkViewport viewport = {};
	viewport.height = (float)extent.height;
	viewport.width = (float)extent.width;

	viewport.maxDepth = 1.0f;

	VkRect2D rect = {};
	rect.extent.width = (float)extent.width;;
	rect.extent.height = (float)extent.height;;
	rect.offset = { 0,0 };

	vkCmdSetViewport(imGuiCmds[imageIndex], 0, 1, &viewport);
	vkCmdSetScissor(imGuiCmds[imageIndex], 0, 1, &rect);

	renderpass->passes["INTERFACE"]->beginRenderPass(imGuiCmds[imageIndex], framebuffersManager->framebuffers["INTERFACE"][imageIndex]->getFramebufferHandle());

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();

	//UI Desing
	ImGui::Begin("Kalm");
	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

	if (ImGui::BeginTabBar("Inspector", tab_bar_flags))
	{
			if (ImGui::BeginTabItem("Scene Graph"))
			{
			

				ImGui::InputFloat4("Ortho", (float*)glm::value_ptr(ortho));

				ImGui::InputFloat("Dist", &dist);
				ImGui::InputFloat2("Nearfar", (float*)glm::value_ptr(nearFar));

				mainSCene.sceneGraph.buildUI(mainSCene.sceneGraph.root);
				ImGui::EndTabItem();
			
		}
		if (ImGui::BeginTabItem("Scene Settings"))
		{

			ImGui::EndTabItem();
			if (ImGui::Button("Save")) {
				std::fstream saveFile;
				std::fstream saveStateCurrent;
				std::ifstream inputFile("SceneState.txt");
			
				
				saveStateCurrent.open("SceneState.txt", std::ios::out|std::ios::trunc|std::ios::in);

				if (saveStateCurrent.is_open()) {

					mainSCene.sceneGraph.saveState( mainSCene.sceneGraph.root, saveStateCurrent);
					auto cam = main_camera->getComponent(Engine::COMPONENT_TYPE::SCRIPT);
		

					cam->loadState(saveStateCurrent);

					saveStateCurrent.close();

				}

				
				if (ImGui::BeginPopupModal("Save to File", NULL)) {
					ImGui::Text("Scene State is saved\n");
					ImGui::EndPopup();
				}

			}
		}
		if (ImGui::BeginTabItem("Scene Statistics"))
		{
			ImGui::Separator();
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "PIPELINE STATISTICS");
			ImGui::BulletText(
				"INPUT_ASSEMBLY_VERTICES_BIT = %d\n",

				renderQueries.statistics[0]			
			);
			ImGui::BulletText(
				"NPUT_ASSEMBLY_PRIMITIVES_BIT = %d\n",
				renderQueries.statistics[1]
			);
			ImGui::BulletText(
			
				"VERTEX_SHADER_INVOCATIONS_BIT = %d\n",
				renderQueries.statistics[2]
			);

			ImGui::BulletText(
				"CLIPPING_INVOCATIONS_BIT = %d\n",
			
				renderQueries.statistics[3]
			);

			ImGui::BulletText(

				"CLIPPING_PRIMITIVES_BIT = %d\n",

				renderQueries.statistics[4]
			);

			ImGui::BulletText(
		
				"FRAGMENT_SHADER_INVOCATIONS_BIT = %d\n",

				renderQueries.statistics[5]

			);

			ImGui::Separator();
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "APPROXIMATED TIMESTAMPS");

			ImGui::BulletText(

				"SHADOW_MAP = %f\n",

				(renderQueries.timeStamps[1] - renderQueries.timeStamps[0])*device.getTimeStampPeriod()/1000000.0f

			);		ImGui::BulletText(

				"GBUFFER_COMPOSITION = %f\n",

				(renderQueries.timeStamps[3] - renderQueries.timeStamps[2])* device.getTimeStampPeriod() / 1000000.0f

			);		
			ImGui::BulletText(

				"DEFERRED_LIGHTING = %f\n",

				((renderQueries.timeStamps[5] - renderQueries.timeStamps[4])* device.getTimeStampPeriod() / 1000000.0f)
				 
			);		
			
			ImGui::BulletText(

				"GAUSSIAN_BLUR_HORIZON_VERTICAL = %f\n",

				(renderQueries.timeStamps[7] - renderQueries.timeStamps[6])* device.getTimeStampPeriod() / 1000000.0f

			);		ImGui::BulletText(

				"PRESENTATION = %f\n",

				(renderQueries.timeStamps[9] - renderQueries.timeStamps[8])* device.getTimeStampPeriod() / 1000000.0f

			);

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();

	//ImGui::ShowDemo();

	ImGui::Render();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imGuiCmds[imageIndex]);

	renderpass->passes["INTERFACE"]->endRenderPass(imGuiCmds[imageIndex]);

	Vk_Functions::endCommandBuffer(imGuiCmds[imageIndex]);

}

void Render::recreateSwapChain()
{
	std::cout << "SWAPCHAIN RECREATED\n";
	int w, h = 0;
	glfwGetFramebufferSize(window, &w, &h);
	main_camera->setWidthHeight(w, h);
	while (w == 0 || h == 0) {
		glfwGetFramebufferSize(window, &w, &h);

		glfwWaitEvents();
	}
	vkDeviceWaitIdle(device.getLogicalDevice());

	//Destroy Resources
	swapChain.destroySwapChain(device);
	//Destroy Renderpasses
	renderpass.reset();
	//Destroy Frambuffers
	framebuffersManager.reset();
	//Destroy Pipelines
	//pipelineManager.clear();
	//Reset Command Pool
	vkResetCommandPool(device.getLogicalDevice(), graphicsPool->getPoolHanndle(), VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

	//Get current Window Size
	int width, height;
	glfwGetWindowSize(window,&width, &height);

	//Rebuild STUFF ****
	// 
	//Setup the swapChain
	VK_Objects::QueueSharingMode queueSharingMode = device.getQueueSharingMode();
	VK_Objects::ImageFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	swapChain.prepareSwapChain(width, height, device, &surface, format, window, queueSharingMode);
	//Create First renderpass;
	createRenderpass();
	//Create FrameBuffers
	framebuffersManager = std::make_unique<FramebufferManagement>(&device, &swapChain, renderpass->passes);
	//Create Pipelines
	//createPipeline();
	//Create CommandPools
	AllocateCommonDescriptorsSets();
	createRenderContexts();
}



void Render::updateSceneGraph()
{
	mainSCene.sceneGraph.updateSceneGraph();
}


void Render::updateUniforms(uint32_t imageIndex)
{	glm::vec3 lightDireciton = glm::normalize(lightUniform.lights[0].position);




	lightUniform.invProj = glm::inverse(main_camera->getProjectionMatrix());
	
	lightUniform.camera = std::dynamic_pointer_cast<Engine::Transform>(main_camera->getComponent(Engine::COMPONENT_TYPE::TRANSFORM))->getPosition();
	lightUniform.invView = glm::inverse(main_camera->getViewMatrix());
	//mainLight = light1;

	uint32_t i = 0;
	for (auto light : lightContainer) {
		LightUbo l{};
		l.color = light->getColor();
		l.position = light->getPosition();
		l.typeFactor = glm::vec3(light->getType(), light->getFactor(),.0);
		lightUniform.lights[i] = l;
		i++;
	};
	lightUniform.num_lights = i+1 ;




	//Light Space Directions
	glm::vec3 right = glm::normalize(glm::cross(lightDireciton, glm::vec3(0, 1, 0)));
	glm::vec3 lightUp = glm::vec3(glm::cross(lightDireciton, right));
	glm::vec3 p = lightDireciton * main_camera->getFarPlane() * dist;
	
	glm::mat4 depthViewMatrix = lookAt(lightDireciton *main_camera->getFarPlane() + main_camera->getCenter(), main_camera->getCenter(), lightUp);

	//glm::mat4 depthViewMatrix = lookAt(normalize(lightDireciton*(main_camera->getFarPlane() - main_camera->getNearPlane()) - main_camera->getCenter()),main_camera->getCenter(), glm::vec3(0,-1,0));

	std::array<float, 6> boundingBox = main_camera->calculateFrustumInLightSpace(depthViewMatrix,glm::vec3(lightDireciton));

	glm::mat4 depthProjectionMatrix = glm::ortho(boundingBox[0], boundingBox[1], boundingBox[2], boundingBox[3],nearFar.x,nearFar.y );
	//glm::mat4 depthProjectionMatrix = glm::ortho(ortho.x, ortho.y, ortho.z, ortho.w, nearFar.x, nearFar.y);

	glm::mat4 lightMatrix = depthProjectionMatrix * depthViewMatrix;
	lightUniform.lightMatrix = lightMatrix;

	//main_camera->setFarplane(nearFar.y);
	//main_camera->setNearPlane(nearFar.x);
	
	lightUniformBuffers[imageIndex]->udpate(lightUniform);

	lightProjectionUniformBuffers[imageIndex]->udpate(lightMatrix);

	VP t;

	t.view = main_camera->getViewMatrix();

	t.projection = main_camera->getProjectionMatrix();

	viewProjectionBuffers[imageIndex]->udpate(t);
	//Update the uniform buffers that hold model projection, which is stored in one large buffer per frame.
	updateDynamicUniformBuffer(imageIndex);


}

void Render::updateDynamicUniformBuffer(uint32_t imageIndex)
{


	if (!modelBuffers[imageIndex]->isMapped()) {
		vkMapMemory(device.getLogicalDevice(), modelBuffers[imageIndex]->getMemoryHandle(), 0, modelBuffersSize, 0, &modelBuffers[imageIndex]->mapPointer);
		modelBuffers[imageIndex]->setMapped(true);

	}
	for (int i = 0; i < meshes.size(); i++) {

		if (true) {

			glm::mat4* m0 = (glm::mat4*)((uint64_t)modelMatrixes.model + dynamicAlignment * i);

			glm::mat4 mm = *m0;

			*m0 = meshes[i]->getModelMatrix();


		}

		memcpy(modelBuffers[imageIndex]->mapPointer, modelMatrixes.model, modelBuffersSize);

		// Flush to make changes visible to the host
		VkMappedMemoryRange memoryRange{};
		memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		memoryRange.memory = modelBuffers[imageIndex]->getMemoryHandle();
		memoryRange.size = modelBuffersSize;
		memoryRange.offset = 0;

		vkFlushMappedMemoryRanges(device.getLogicalDevice(), 1, &memoryRange);
	}
}

Render::~Render()
{
	cubeMap.reset();
	envMAp.reset();
	brdfLut.reset();
	irradianceMap.reset();

	for (auto& b : modelBuffers) {
		vkUnmapMemory(device.getLogicalDevice(), b->getMemoryHandle());
	}

	vkDeviceWaitIdle(device.getLogicalDevice());
	ImGui_ImplVulkan_Shutdown();


	vkDestroySampler(device.getLogicalDevice(), sampler_streatch, nullptr);
	vkDestroySampler(device.getLogicalDevice(), sampler_Testing, nullptr);


	//(device.getLogicalDevice(), sampler, device.getAllocator());

	//Destroy Debuger
	debuger.destroy(instance);
	//Destroy SwapChain
	swapChain.destroySwapChain(device);
	//Destroy renderpass
	Game::RenderPasses::iterator it = renderpass->passes.begin();

	for (int i = 0; i < viewProjectionBuffers.size(); i++) {
		viewProjectionBuffers[i].reset();
		lightUniformBuffers[i].reset();
		lightProjectionUniformBuffers[i].reset();
	}

	transferPool.reset();
	graphicsPool.reset();
	dynamicPool.reset();
	poolManager.reset();
	framebuffersManager.reset();
	renderpass.reset();
	vkDestroyQueryPool(device.getLogicalDevice(), renderQueries.statisticPool, device.getAllocator());
	vkDestroyQueryPool(device.getLogicalDevice(), renderQueries.timeStampsPool, device.getAllocator());

	for (auto& mesh : meshes) {
		mesh->destroy();
	}

	for (auto& buffer : modelBuffers) {
		buffer.reset();
	}

	std::unordered_map<std::string, std::unique_ptr<Engine::Material>>::iterator itM = materialManager.begin();

	while (itM != materialManager.end()) {

		itM->second.reset();
		itM++;
	}


	std::unordered_map<const char*, std::unique_ptr<VK_Objects::Pipeline>>::iterator itP = pipelineManager.begin();

	while (itP != pipelineManager.end()) {
		itP->second.reset();
		itP++;
	}


	//Destroy Device
	device.destroyLogicDevice();
	//Destroy surface
	vkDestroySurfaceKHR(instance.vk_Instance, surface, nullptr);
	//Destroy Instance

	instance.destroy();

}

void Render::perfomCulling()
{
	Engine::Frustum  frustum =  main_camera->calculateFrustumPlanes();
	performCullingOnSceneGraph(mainSCene.sceneGraph.root,frustum);

}

void Render::performCullingOnSceneGraph(std::shared_ptr<Node> node,Engine::Frustum& frustum)
{

	std::list<std::shared_ptr<Node>>::iterator it = mainSCene.sceneGraph.root->childs.begin();

	while (it != mainSCene.sceneGraph.root->childs.end()) {

		if ((it->get()->parent->activated == true)) {


			glm::vec3 position = it->get()->entity->transform->getPosition();

			if (glm::dot(position, frustum.nearPlane.normal) - frustum.nearPlane.distance < -1.0f) { mainSCene.sceneGraph.setActivatedChilds(*it, false); it++; continue; }
			if (glm::dot(position, frustum.farPlane.normal) - frustum.farPlane.distance < -1.0f) { mainSCene.sceneGraph.setActivatedChilds(*it, false); it++; continue; }
			if (glm::dot(position, frustum.rightPlane.normal) - frustum.rightPlane.distance < -1.0f) { mainSCene.sceneGraph.setActivatedChilds(*it, false); it++; continue; }
			if (glm::dot(position, frustum.leftPlane.normal) - frustum.leftPlane.distance < -1.0f) { mainSCene.sceneGraph.setActivatedChilds(*it, false); it++; continue; }
			if (glm::dot(position, frustum.topPlane.normal) - frustum.topPlane.distance < -1.0f) { mainSCene.sceneGraph.setActivatedChilds(*it, false); it++; continue; }
			if (glm::dot(position, frustum.bottomPlane.normal) - frustum.bottomPlane.distance < -1.0f) { mainSCene.sceneGraph.setActivatedChilds(*it, false); it++; continue; }
			mainSCene.sceneGraph.setActivatedChilds(*it, true);
			it++;

		}
		
	}
	

}
