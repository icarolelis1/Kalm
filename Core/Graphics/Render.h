#pragma once
#include "VulkanFramework.h"
#include "Debuger.h"
#include <sstream>
#include "GLFW/glfw3.h"
#include "RenderContext/RenderContext.h"
#include <Graphics/Cubemap/CubeMap.h>
#include <memory>
#include <chrono>
#include <Graphics/FrameBuffer/Framebuffer.h>
#include <Graphics/GraphicsUtil/GraphicsUtility.h>
#include <MeshComponent/Mesh.h>
#include <cameraController/CameraController.h>
#include <Light/Light.h>
#include <ScriptManager/ScriptManager.h>
#include <Graphics/RenderpassManager/RenderpassManager.h>
#include <Graphics/FramebufferManager/FramebufferManagement.h>
#include <Scene/Scene.h>
#include <Graphics/Thread/Thread.h>
#include <Collisor/sphere/SphereCollisor.h>
#include <fstream>

#ifndef GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#endif

constexpr bool DEBUG_ = true;
constexpr bool UI_RENDER = true;

using Meshes = std::vector<std::shared_ptr<Engine::Mesh>>;
using LightContainer = std::vector<std::shared_ptr<Engine::LightComponent>>;



struct VP {

	 glm::mat4 view;
	 glm::mat4 projection;
	 float time;
};

struct ModelMatrix {
	glm::mat4* model = nullptr;
};

struct LightUbo {

	alignas(16) glm::vec3 color;
	alignas(16) glm::vec3 position;
	alignas(16)	glm::vec3 typeFactor;
};
struct LightUniform {

	glm::mat4 invView;
	glm::mat4 invProj;
	alignas(16) glm::vec3 camera;
	LightUbo lights[3];
	glm::mat4 lightMatrix;
	int num_lights = 1;
};

class Render
{
	struct TimeStep {

		TimeStep(float t) :time(t) {};
		float time;
		float getTimeInSec() { return time; };
		float getTimeinMilli() { return (time * 1000.); };
	};


public:
	Render();
	glm::vec2 nearFar = glm::vec2(.1f,103.f);
	glm::vec4 ortho = glm::vec4(10,-10,-10,10);
	float dist = 40;
	void initiateResources(Utils::WindowHandler *windowHandler, uint32_t WIDTH, uint32_t HEIGHT);


	~Render();

private:
	void createInstance();
	void createDebuger();
	void createSurface(Utils::WindowHandler*);
	void prepareDevice(VK_Objects::Surface surface);
	void createRenderpass();
	void createRenderContexts();
	void createShadowMap(VkCommandBuffer &commandBuffer,uint32_t imageIndex);
	void createBloom(VkCommandBuffer& commandBuffer, uint32_t imageIndex);

	void createPipeline();
	void creteCommandBuffer();

	void createCommandPools();
	void addMeshes();
	void createDynamicUniformBuffers();
	void createMaterials();

	void createScene();

	void separateSceneObjects(std::shared_ptr<Node> node);
	void getEntityScripts();
	void getEntityMeshes();
	void createImGuiInterface();

	void renderUI(uint32_t imageIndex);

	void recreateSwapChain();
	VK_Objects::Instance instance;
	VK_Objects::Device device;
	VK_Objects::Surface surface;
	VK_Objects::SwapChain swapChain;
	VK_Objects::SDescriptorPoolManager poolManager;
	VK_Objects::SDescriptorPoolManager dynamicDEscriptorPoolManager;


	std::vector<VK_Objects::SBuffer> viewProjectionBuffers;
	std::vector<VK_Objects::SBuffer> modelBuffers;
	std::vector<VK_Objects::SBuffer> lightUniformBuffers;
	std::vector<VK_Objects::SBuffer> lightProjectionUniformBuffers;

	Utils::WindowHandler* w;
	
	size_t modelBuffersSize;
	VK_Objects::PCommandPool graphicsPool;
	VK_Objects::PCommandPool transferPool;
	VK_Objects::PCommandPool dynamicPool;

	std::shared_ptr<Engine::Camera> main_camera;


	VkSampler sampler_Testing;
	VkSampler sampler_streatch;

	std::vector<VK_Objects::Descriptorset> globalData_Descriptorsets;
	std::vector<VK_Objects::Descriptorset> modelMatrix_Descriptorsets;
	std::vector<VK_Objects::Descriptorset> deferredShading_Descriptorsets;
	std::vector<VK_Objects::Descriptorset> enviromentData_Descriptorsets;
	std::vector<VK_Objects::Descriptorset> verticalBlur_Descriptorsets;
	std::vector<VK_Objects::Descriptorset> horizontalBlur_Descriptorsets;
	std::vector<VK_Objects::Descriptorset> finalOutPut_Descriptorsets;


	std::vector<VK_Objects::Descriptorset> light_Descriptorsets;
	std::vector<VK_Objects::Descriptorset> lightProjection_Descriptorset;
	size_t dynamicAlignment;

	Utils::Debuger debuger;

	//RenderPasses renderpasses;

	std::vector<VkCommandBuffer> imGuiCmds;
	Engine::ScriptManager scriptManager;
	std::unique_ptr<Game::RenderpassManager> renderpass;
	MaterialManager materialManager;

	LightUniform lightUniform;

	void updateSceneGraph();
	void updateUniforms(uint32_t imageIndex);
	void updateDynamicUniformBuffer(uint32_t imageIndex);
	Game::Scene mainSCene;
	glm::vec3 mainLightPos = glm::vec3(400);
	
	LightContainer lightContainer;

	float lastFrameTIme = 0;
	float frameTime;
	PipelineManager pipelineManager;
	std::unique_ptr<FramebufferManagement> framebuffersManager;
	Meshes meshes;
	Utils::WindowHandler* window;
	ModelMatrix modelMatrixes;

};

