#pragma once
#include "VulkanFramework.h"
#include <vector>

namespace Utils {

	inline VkResult  CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	
	inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}


	class Debuger {

	public:
		Debuger() { debugMessenger = 0; };

		const std::vector<const char*> getValidationLayers();
		const std::vector<const char*> getExtension(bool enableValidationLayers);

		void setDebugerMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo, VK_Objects::Instance instance);
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		bool checkLayerSupport();


		void destroy(VK_Objects::Instance instance);
		~Debuger() {};

	private:
		VkDebugUtilsMessengerEXT debugMessenger;


		//Debug messenger callback function
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) {

			std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		}


	};

	


}
