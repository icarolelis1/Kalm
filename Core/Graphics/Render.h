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
#include <ScriptManager/ScriptManager.h>
#include <Graphics/RenderpassManager/RenderpassManager.h>
#include <Graphics/FramebufferManager/FramebufferManagement.h>
#include <Scene/Scene.h>
#include <Graphics/Thread/Thread.h>
#ifndef GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#endif

constexpr bool DEBUG_ = true;

using Meshes = std::vector<std::shared_ptr<Engine::Mesh>>;

using PipelineManager = std::unordered_map<const char*, std::unique_ptr<VK_Objects::Pipeline>>;

using MaterialManager = std::unordered_map<const char*, std::unique_ptr<Engine::Material>>; 

struct VP {

	 glm::mat4 view;
	 glm::mat4 projection;
	 float time;
};

struct ModelMatrix {
	glm::mat4* model = nullptr;
};

struct LightUbo {

	alignas(16) glm::vec3 position;
	alignas(16) glm::vec3 color;
	float type;

};
struct LightUniform {

	glm::mat4 invView;
	glm::mat4 invProj;
	LightUbo light[1];

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
	void createPipeline();
	void creteCommandBuffer();

	void createCommandPools();
	void addMeshes();
	void createDynamicUniformBuffers();
	void createMaterials();

	void createScene();

	void separateSceneObjects();
	void getEntityScripts();
	void getEntityMeshes();

	VK_Objects::Instance instance;
	VK_Objects::Device device;
	VK_Objects::Surface surface;
	VK_Objects::SwapChain swapChain;
	VK_Objects::SDescriptorPoolManager poolManager;
	std::vector<VK_Objects::SBuffer> viewProjectionBuffers;
	std::vector<VK_Objects::SBuffer> modelBuffers;
	std::vector<VK_Objects::SBuffer> lightUniformBuffers;
	std::vector<VK_Objects::SBuffer> lightProjectionUniformBuffers;

	Utils::WindowHandler* w;
	
	size_t modelBuffersSize;
	VK_Objects::PCommandPool graphicsPool;
	VK_Objects::PCommandPool transferPool;

	Engine::Camera camera;
	LightUbo mainLight;


	VkSampler sampler_Testing;

	std::vector<VK_Objects::Descriptorset> globalData_Descriptorsets;
	std::vector<VK_Objects::Descriptorset> modelMatrix_Descriptorsets;
	std::vector<VK_Objects::Descriptorset> deferredShading_Descriptorsets;
	std::vector<VK_Objects::Descriptorset> light_Descriptorsets;
	std::vector<VK_Objects::Descriptorset> lightProjection_Descriptorset;
	size_t dynamicAlignment;

	Utils::Debuger debuger;

	//RenderPasses renderpasses;

	Engine::ScriptManager scriptManager;
	std::unique_ptr<Game::RenderpassManager> renderpass;
	MaterialManager materialManager;


	void updateUniforms(uint32_t imageIndex);
	void updateDynamicUniformBuffer(uint32_t imageIndex);
	Game::Scene mainSCene;


	float lastFrameTIme = 0;
	float frameTime;
	PipelineManager pipelineManager;
	std::unique_ptr<FramebufferManagement> framebuffersManager;
	Meshes meshes;
	Utils::WindowHandler* window;
	ModelMatrix modelMatrixes;

};

