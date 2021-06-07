#include "Debuger.h"
#include "GLFW/glfw3.h"

namespace Utils {



	const std::vector<const char*> Debuger::getValidationLayers() {
		return VK_Objects::validationLayers;
	}

	const std::vector<const char*> Debuger::getExtension(bool enableValidationLayers) {

		uint32_t	 glfwExtensionsCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

		if (enableValidationLayers) {

			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);  // DISPLAY WARNING\ERROR MESSAGES

		}

		return extensions;
	}

	bool Debuger::checkLayerSupport() {

		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> avaibleLayers;
		avaibleLayers.resize(layerCount);

		vkEnumerateInstanceLayerProperties(&layerCount, avaibleLayers.data());

		//Check if all layer inside validationLayer are supported (inside avaibleLayers).

			//Will tag if it can't find any layer;
		bool anyMyssingLayer = 0;
		for (const char* layerName : VK_Objects::validationLayers) {
			bool foundLayer = 0;

			for (const auto& layerProperties : avaibleLayers) {

				if (strcmp(layerProperties.layerName, layerName) == 0) {
					foundLayer = 1;
					break;
				}

			}

			if (foundLayer == 0) {
				anyMyssingLayer = 1;

			}

		}

		if (anyMyssingLayer) {
			std::cout << "    LAYER NOT SUPPORTED \n";
			return false;
		}

		return true;

	}

	void Debuger::setDebugerMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo, VK_Objects::Instance instance) {

		populateDebugMessengerCreateInfo(createInfo);

		if (Utils::CreateDebugUtilsMessengerEXT(instance.vk_Instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			std::cout << "    Failed to create DebugMessenger\n";
		}
		
	
	}

	void Debuger::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}
	void Debuger::destroy(VK_Objects::Instance instance)
	{
		DestroyDebugUtilsMessengerEXT(instance.vk_Instance, debugMessenger, nullptr);
		std::cout << "    Debuger Destroyed\n";
	}
}