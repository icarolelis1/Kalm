#pragma once
#include <iostream>
#include "VulkanDef.h"
#include <optional>
#include <vector>
#include "GLFW/glfw3.h"
#include <set>
#include <algorithm>
#include <fstream>
#include <array>

//Graphics FrameWork for vulkan abstractions like Textures, Pipelines, Renderpasses and etc
namespace VK_Objects {

	// STANDARD LUNGARG validation layer
	static const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation",


	};

	using Surface = VkSurfaceKHR;

	using SurfaceFormat = VkSurfaceFormatKHR;

	using ImageFormat = VkFormat;

	struct SwapchainQueryProperties {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentationModes;
	};

	struct SwapchainProperties {

		VkSurfaceFormatKHR format;
		VkPresentModeKHR presentMode;
		VkSurfaceCapabilitiesKHR capabilities;
		uint32_t minImageCount;
		VkExtent2D extent;
		VkSharingMode sharingMode;
		VkSwapchainKHR vk_swapChain;

	};


	struct QueueSharingMode {

		VkSharingMode sharingMode;
		uint32_t queueFamiliyIndexCount;
		const uint32_t* queueFamilies;
	};

	struct RenderImages {

		std::vector<VkImage> vk_images;
		std::vector<VkImageView> vk_imageViews;
		std::vector<VkDeviceMemory>  vk_deviceMemories;

		void destroy(VkDevice device) {
			for(auto &image: vk_images)
			vkDestroyImage(device,image ,nullptr);
			for(auto &view : vk_imageViews)
			vkDestroyImageView(device, view, nullptr);
			for(auto &memory: vk_deviceMemories)
			vkFreeMemory(device, memory, nullptr);
		}
	};

	struct RenderAttachment {

		VkAttachmentDescription description;
		VkAttachmentReference reference;


	};

	struct RenderpassProperties {

		std::vector<RenderAttachment> attachments;

	};

	struct Subpass {

		std::vector<VkSubpassDependency> dependencies;
		std::vector<VkSubpassDescription> description;
	};

	class Instance {

	public:

		VkInstance vk_Instance;

		void destroy();


	};

	enum class  QUEUE_TYPE {
		GRAPHICS,
		PRESENT,
		TRANSFER

	};

	class Device {

		struct QueueFamilyIndices {
			//Queue Family Indices
			std::optional<uint32_t> transfer;
			std::optional<uint32_t> graphics;
			std::optional<uint32_t> compute;
			std::optional<uint32_t> present;
			
			bool isComplete() {
				checkGraphicsQueue();
				checkTransferQueue();
				checkComputeQueue();

				return graphics.has_value() * transfer.has_value() * compute.has_value() * transfer.has_value();
			}

			bool checkGraphicsQueue() {

				if (!graphics.has_value()) {
					std::cout << "\tThere is no GraphicsQueue Suitable\n";
					return false;
				}
				std::cout << "\tThe Graphics Queue is at Index : " << graphics.value() << std::endl;
				return true;
			}

			bool checkTransferQueue() {
				if (!transfer.has_value()) {
					std::cout << "\nThere is no TransferQueue Suitable\n";
					return false;
				}
				std::cout << "\tThe Transfer Queue is at Index : " << transfer.value() << std::endl;

				return true;
			}

			bool checkComputeQueue() {
				if (!compute.has_value()) {
					std::cout << "\nThere is no Compute Suitable\n";
					return false;
				}
				std::cout << "\tThe Compute Compute is at Index : " << compute.value() << std::endl;

				return true;
			}

			bool checkPresentQueue() {
				if (!transfer.has_value()) {
					std::cout << "\nThere is no Presentation Suitable\n";
					return false;
				}
				std::cout << "\tThe Presentation Queue is at Index : " << present.value() << std::endl;

				return true;
			}

		};

		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

	public:
		Device() {};


		void choosePhysicalDevice(VK_Objects::Instance instance, VK_Objects::Surface surface) {

			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance.vk_Instance, &deviceCount, nullptr);

			if (deviceCount == 0) { std::cout << "    Failed to find a proper Graphics Card\n"; };

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance.vk_Instance, &deviceCount, devices.data());

			//Score of the GPU 
			VkBool32 findSuitableGPU = false;

			int currentScore = 0;
			int bestScore = 0;
			VkPhysicalDevice bestDevice = NULL;

			for (const auto& device : devices) {

				VkPhysicalDeviceProperties physicalDeviceProps;
				VkPhysicalDeviceFeatures features;
				VkPhysicalDeviceMemoryProperties props;

				//Query physical device properties
				vkGetPhysicalDeviceProperties(device, &physicalDeviceProps);
				vkGetPhysicalDeviceMemoryProperties(device, &props);
				vkGetPhysicalDeviceFeatures(device, &features);


				currentScore = scorePhysicalDevice(device, features, props, surface);
				//currentScore >=100. means that the device have all the minimum demanded capabilities
				if (currentScore >= 100. && !findSuitableGPU) {
					findSuitableGPU = true;
					bestDevice = device;
					bestScore = currentScore;
				}
				//If we find any better device we pick it
				if (currentScore > bestScore) {
					bestDevice = device;
					bestScore = currentScore;

				}

			}

			if (!findSuitableGPU) {

				std::cout << "    Failed to find a  Graphics Card that suits the requirements\n";
				return;
			}

			else {

				vk_physicalDevice = bestDevice;
				//Query physical device properties
				vkGetPhysicalDeviceProperties(vk_physicalDevice, &vk_physicalDeviceProperties);
				vkGetPhysicalDeviceMemoryProperties(vk_physicalDevice, &vk_MemoryProperties);
				vkGetPhysicalDeviceFeatures(vk_physicalDevice, &vk_PhysicalDevicefeatures);

				std::cout << "Device : \n";
				std::cout << "   " << vk_physicalDeviceProperties.deviceName << std::endl;



				vk_physicalDevice = bestDevice;
				msaaSamples = getMaxUsableSampleCount();
				std::cout << msaaSamples << std::endl;

			}



		};

		size_t getMinimumBufferAligment() {
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(vk_physicalDevice, &props);
			return props.limits.minUniformBufferOffsetAlignment;
		}

		VkSampleCountFlagBits getMaxUsableSampleCount() {
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(vk_physicalDevice, &physicalDeviceProperties);

			VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
			if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
			if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
			if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
			if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
			if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
			if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

			return VK_SAMPLE_COUNT_1_BIT;
		}

		int scorePhysicalDevice(VkPhysicalDevice device, VkPhysicalDeviceFeatures features, VkPhysicalDeviceMemoryProperties memProperties, Surface surface) {

			/*this is not really good yet ,
			But for now It just take a suitable device with the largest heap
			*/
			int score = 200;

			auto properties = VkPhysicalDeviceProperties{};
			vkGetPhysicalDeviceProperties(device, &properties);

			uint32_t queueFamilityCount = 0;

			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilityCount, nullptr);
			std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilityCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilityCount, queueFamilyProperties.data());

			VkBool32 foundGraphics = false;
			VkBool32 foundCompute = false;
			VkBool32 foundTransfer = false;
			VkBool32 foundPresent = false;

			if (properties.deviceType & VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score -= 50;
			else {

				score -= 100;
			}

			for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {

				if (!foundGraphics && getQueuFamilieIndex(queueFamilyProperties[i], VK_QUEUE_GRAPHICS_BIT)) {
					foundGraphics = true;
					queueFamilies.graphics = i;
				}
				if (!foundCompute && getQueuFamilieIndex(queueFamilyProperties[i], VK_QUEUE_COMPUTE_BIT)) {
					foundCompute = true;
					queueFamilies.compute = i;

				}
				if (!foundTransfer && getQueuFamilieIndex(queueFamilyProperties[i], VK_QUEUE_TRANSFER_BIT)) {
					foundTransfer = true;
					queueFamilies.transfer = i;

				}

				if (!foundPresent)
					vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &foundPresent);

				if (foundPresent && !queueFamilies.present.has_value()) {
					queueFamilies.present = i;
				}

			}

			//if device has no graphics neither presentation capabilities, the device is not suitable (score = 0)
			if (!foundGraphics) score = 0;
			if (!foundPresent)  score = 0;
			if (!foundTransfer) score -= 20; //means that the device doesn't have dedicated queue to transfer operations
			if (!foundCompute)  score -= 20; //means that the device doesn't have dedicated queue to compute operations

			if (!checkDeviceExtensions(device)) score = 0; //check if the device have the required extensions
			if(!querySwapChainProperties(device,surface))score = 0;//check if the device is suitable for the swapChain

			if (memProperties.memoryHeaps->size > 2000)score += 10;
			if (memProperties.memoryHeaps->size > 4000)score += 10;
			if (memProperties.memoryHeaps->size > 8000)score += 10;

			bool featuresQuery = features.samplerAnisotropy && features.shaderClipDistance && features.fillModeNonSolid;

			if (featuresQuery)score += 10;

			return score;

		}

		void createLogicalDevice() {

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t>uniqueFamilies = { queueFamilies.graphics.value(),queueFamilies.present.value() ,queueFamilies.transfer.value() };


			for (uint32_t queueFamily : uniqueFamilies) {
				VkDeviceQueueCreateInfo QcreateInfo{};
				QcreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				QcreateInfo.queueCount = 1;
				QcreateInfo.queueFamilyIndex = queueFamily;
				float priority = 1.0f;
				QcreateInfo.pQueuePriorities = &priority;
				queueCreateInfos.push_back(QcreateInfo);
			}



			VkPhysicalDeviceFeatures features{};
			features.samplerAnisotropy = VK_TRUE;
			features.fillModeNonSolid = VK_TRUE;

			VkDeviceCreateInfo deviceInfo = {};
			deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
			deviceInfo.pEnabledFeatures = &features;
			deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
			deviceInfo.enabledLayerCount = static_cast<uint32_t>(VK_Objects::validationLayers.size());
			deviceInfo.ppEnabledLayerNames = VK_Objects::validationLayers.data();

			VkResult result = vkCreateDevice(vk_physicalDevice, &deviceInfo, nullptr, &vk_Device);

			if (result != VK_SUCCESS) {
				std::cout << "    Failed to create Logical Device\n";
			}

			if (result == VK_SUCCESS) {
				std::cout << "\nSucessfully created a logical device \n";
				queueFamilies.isComplete();
			}
			vkGetDeviceQueue(vk_Device, queueFamilies.graphics.value(), 0, &graphicsQueue);
			vkGetDeviceQueue(vk_Device, queueFamilies.present.value(), 0, &presentationQueue);
			vkGetDeviceQueue(vk_Device, queueFamilies.transfer.value(), 0, &transferQueue);

		}

		void destroyLogicDevice() {

		
			vkDestroyDevice(vk_Device, nullptr);
			std::cout << "    SuccessFully destroyed Device\n";

		}

		const VkPhysicalDevice getPhysicalDevice() const {
			return vk_physicalDevice;
		}

		const VkDevice getLogicalDevice()const  {
			return vk_Device;
		}

		const QueueSharingMode getQueueSharingMode() const{

			//This function checks if the presentation Queue Family is different of Graphics Queue Family. This is relevant for the swapChain
			//Creation

			QueueSharingMode queueSharing;
			if (queueFamilies.graphics.value() != queueFamilies.present.value()) {

				queueSharing.sharingMode = VK_SHARING_MODE_CONCURRENT;
				queueSharing.queueFamiliyIndexCount = 2;
				std::vector<uint32_t> queueF(2);
				queueF = { queueFamilies.graphics.value(),queueFamilies.present.value() };
				queueSharing.queueFamilies = queueF.data();
				return queueSharing;

			}

			queueSharing.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			queueSharing.queueFamiliyIndexCount = 1;
			return queueSharing;
		}

		const uint32_t getGraphicsQueueIndex()const {

			return queueFamilies.graphics.value();
		}

		const uint32_t getComputeQueueIndex()const {

			return queueFamilies.compute.value();
		}

		const uint32_t getTransferQueueIndex()const {

			return queueFamilies.transfer.value();
		}

		const VkQueue& getQueueHandle(QUEUE_TYPE)const ;

		const VkAllocationCallbacks * getAllocator()const { return nullptr; }

		VkFormat getSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)const;


	private:

		bool checkDeviceExtensions(VkPhysicalDevice device) {
		
			uint32_t extensionsCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

			std::vector<VkExtensionProperties> extensions(extensionsCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensions.data());

			std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());


			for (const auto& extension : extensions) {
				requiredExtensions.erase(extension.extensionName);
			}

			return requiredExtensions.empty();

		};

		bool getQueuFamilieIndex(VkQueueFamilyProperties props, VkQueueFlagBits queueFlags)
		{

			if (queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				if (props.queueFlags & queueFlags) {
					return true;

				}
			}

			else if (queueFlags & VK_QUEUE_COMPUTE_BIT) {

				if (props.queueFlags & queueFlags)	return true;

			}

			else if (queueFlags & VK_QUEUE_TRANSFER_BIT) {

				if (props.queueFlags & VK_QUEUE_TRANSFER_BIT && !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT))	return true;

			}

			return false;
		}

		bool querySwapChainProperties(VkPhysicalDevice device,Surface surface) {

			SwapchainQueryProperties properties;
			uint32_t formatCount;
			uint32_t presentModeCount;

			//vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, *surface, &swapChainExt.capabilities);

			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
			properties.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, properties.formats.data());

			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
			properties.presentationModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, properties.presentationModes.data());

			bool isSuitableSwapChain = false;
			isSuitableSwapChain = !properties.formats.empty() && !properties.presentationModes.empty();

			return isSuitableSwapChain;
		}

		VkPhysicalDevice vk_physicalDevice = NULL;
		VkDevice vk_Device = NULL;

		//GPU QUEUES
		VkQueue graphicsQueue;
		VkQueue presentationQueue;
		VkQueue transferQueue;

		QueueFamilyIndices queueFamilies;
		VkPhysicalDeviceProperties vk_physicalDeviceProperties;
		VkPhysicalDeviceMemoryProperties vk_MemoryProperties;
		VkPhysicalDeviceFeatures vk_PhysicalDevicefeatures;

		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;


	};

	class SwapChain {

	
	public:
		SwapChain() {};

		SwapChain(uint32_t WIDTH, uint32_t HEIGHT, Device device, Surface surface, ImageFormat& desiredFormat, GLFWwindow* window, QueueSharingMode& queueSharingMode) {
			properties.extent.width = WIDTH;
			properties.extent.height = HEIGHT;

			querySwapChainProperties(device, surface, desiredFormat, window);
			createSwapChain(device, surface, queueSharingMode);
			aquireSwapChainImages(device);
			createSwapchainViews(device);
		}

		void prepareSwapChain(uint32_t WIDTH, uint32_t HEIGHT, Device device, Surface *surface, ImageFormat& desiredFormat, GLFWwindow* window, QueueSharingMode& queueSharingMode) {

			properties.extent.width = WIDTH;
			properties.extent.height = HEIGHT;

			querySwapChainProperties(device, *surface, desiredFormat, window);
			createSwapChain(device, *surface, queueSharingMode);
			aquireSwapChainImages(device);
			createSwapchainViews(device);
		}

		void destroySwapChain(Device device) {

			for (auto view : swapChainImages.vk_imageViews) {
				vkDestroyImageView(device.getLogicalDevice(), view, nullptr);
			}


			vkDestroySwapchainKHR(device.getLogicalDevice(), properties.vk_swapChain, nullptr);
			std::cout << "    Successfully destroyed SwapChain\n";

		}

		std::vector<VkImage> getImages();

		std::vector<VkImageView> &getViews();

		ImageFormat getSwapchainFormat();

		VkExtent2D getExtent();

		uint32_t getNumberOfImages();

		VkFormat getFormat();

		VkSwapchainKHR& getSwapChainHandle() {return properties.vk_swapChain; } ;


	private:

		void createSwapChain( Device device,Surface &surface, QueueSharingMode& queueSharingMode) {

			VkSwapchainCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface;
			createInfo.clipped = VK_TRUE;
			createInfo.imageFormat = properties.format.format;
			createInfo.minImageCount = properties.minImageCount;
			createInfo.imageArrayLayers = 1;
			createInfo.imageColorSpace = properties.format.colorSpace;
			createInfo.imageSharingMode = properties.sharingMode;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = properties.presentMode;
			createInfo.imageExtent = properties.extent;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			createInfo.preTransform = properties.capabilities.currentTransform;

			if (queueSharingMode.queueFamiliyIndexCount > 1) {
				createInfo.imageSharingMode = queueSharingMode.sharingMode;
				createInfo.queueFamilyIndexCount = queueSharingMode.queueFamiliyIndexCount;
				createInfo.pQueueFamilyIndices = queueSharingMode.queueFamilies;
			}

			else {
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			}


			createInfo.oldSwapchain = VK_NULL_HANDLE;
			createInfo.flags = 0;

			VkResult result = vkCreateSwapchainKHR(device.getLogicalDevice(), &createInfo, nullptr, &properties.vk_swapChain);

			std::cout << "Sucessfully created a SwapChain\n";

		}

		void aquireSwapChainImages( Device device) {

			uint32_t imageCount;
			vkGetSwapchainImagesKHR(device.getLogicalDevice(), properties.vk_swapChain, &imageCount, nullptr);

			swapChainImages.vk_images.resize(imageCount);

			vkGetSwapchainImagesKHR(device.getLogicalDevice(), properties.vk_swapChain, &imageCount, swapChainImages.vk_images.data());
		}

		void createSwapchainViews(Device device);

		bool querySwapChainProperties( Device device,VK_Objects::Surface surface, ImageFormat& desiredFormat, GLFWwindow* window) {

			SwapchainQueryProperties propertiesQuery{};

			uint32_t formatCount;
			uint32_t presentationCount;
			//Surface Cabalities
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getPhysicalDevice(), surface, &properties.capabilities);
			//SUrface supported formats
			vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDevice(), surface, &formatCount, nullptr);
			propertiesQuery.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDevice(), surface, &formatCount, propertiesQuery.formats.data());
			//Surface supported Presentation Modes
			vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice(), surface, &presentationCount, nullptr);
			propertiesQuery.presentationModes.resize(presentationCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice(), surface, &presentationCount, propertiesQuery.presentationModes.data());

			properties.format = chooseFormat(propertiesQuery.formats, desiredFormat);;
			properties.extent = chooseImageExtent(properties.capabilities, window);
			properties.presentMode = choosePresentationMode(propertiesQuery.presentationModes);

			properties.minImageCount = properties.capabilities.minImageCount + 1;

			if (properties.capabilities.maxImageCount > 0 && properties.minImageCount > properties.capabilities.maxImageCount) {
				properties.minImageCount = properties.capabilities.maxImageCount;
			}

			bool isSuitableSwapChain = false;
			isSuitableSwapChain = !propertiesQuery.formats.empty() && !propertiesQuery.presentationModes.empty();

			return isSuitableSwapChain;

		}

		VkSurfaceFormatKHR chooseFormat(std::vector<VkSurfaceFormatKHR> formats, ImageFormat& desiredFormat) {
			//We try to get the desiredFormat 
			for (const auto& format : formats) {
				if (format.format == desiredFormat && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					return format;
			}
			//If we don't get the first format, we take the first one.
			return formats[0];
		}

		VkExtent2D chooseImageExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {

			if (capabilities.currentExtent.width != UINT32_MAX) {
				return capabilities.currentExtent;

			}
			else {
				int w, h;
				VkExtent2D actualExtent;
				//	= { WIDTH, HEIGHT };
				glfwGetFramebufferSize(window, &w, &h);
				actualExtent.width = static_cast<uint32_t>(w);
				actualExtent.height = static_cast<uint32_t>(h);

				return actualExtent;
			}
		}

		VkPresentModeKHR choosePresentationMode(std::vector< VkPresentModeKHR>& presentModes) {
			//We look for mailBox which is the one with highest FPS
			for (const auto& presentMode : presentModes) {
				if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
					std::cout << "\nPRESENTATION MODE : VK_PRESENT_MODE_MAILBOX_KHR \n";
					return presentMode;
				}
			}
			std::cout << "PRESENTATION MODE : VK_PRESENT_MODE_MAILBOX_KHR \n";

			//If we can't get MailBox we go with FIFO 
			return VK_PRESENT_MODE_FIFO_KHR;
		}

		SwapchainProperties properties;

	    RenderImages swapChainImages;
	};

	class Buffer {

	public:

		Buffer( const Device *_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties );

		Buffer(Buffer& other) {
			std::cout << "Copying from :" << id << std::endl;
			std::cout << "To  :" << other.id << std::endl;

			vk_buffer = other.vk_buffer;
		
		};

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		bool isMapped();

		VkBuffer& getBufferHandle();

		VkDeviceMemory& getMemoryHandle();
		
		template<typename T> void udpate(T& object);

		~Buffer();
		void* mapPointer;

		void setMapped(bool m);

		const char* id;
	private:
		bool mapped = 0;
		const Device* device;
		VkBuffer vk_buffer;
		VkDeviceMemory vk_deviceMemory;
	};
	using PBuffer = std::unique_ptr<Buffer>;
	using SBuffer = std::shared_ptr<Buffer>;
	class Renderpass {


	public:
		Renderpass(const Device *device ,const char* _key,VkExtent2D _extent);
		void beginRenderPass(VkCommandBuffer& commandBuffer, VkFramebuffer& frameBuffer, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

		void endRenderPass(VkCommandBuffer cmd);

		VkRenderPass vk_renderpass;
		RenderpassProperties properties;
		Subpass subpass;

		const char* getKey();

		std::vector<VkClearValue> clearValues;

		void destroyRenderpass(const VK_Objects::Device& device);

		~Renderpass();

	private:
		VkExtent2D extent;
		const Device* device;
		const char* key;
	};

	using PRenderpass = std::shared_ptr<VK_Objects::Renderpass>; 

	template<typename T>
	inline void Buffer::udpate(T& object)
	{
		//Keep persistent mapped buffers
		if(!mapped)
		vkMapMemory(device->getLogicalDevice(), vk_deviceMemory, 0, sizeof(object), 0, &mapPointer);
		
		memcpy(mapPointer, &object, sizeof(object));
		mapped = true;
	}

}


//Function Implementations


namespace Utils {

	inline void LOG(const char* n) {
		std::cout << n << std::endl;
	}

	using WindowHandler = GLFWwindow;

	inline bool windowShouldClose(WindowHandler * w) {
		return windowShouldClose(w);
	}

	inline std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}
}

