#pragma once

// GLFW / Vulkan
#define GLFW_INCLUDE_VULKAN // Tell GLFW we want to include Vulkan
#include <GLFW/glfw3.h>     // GLFW
// STL
#include <iostream>  // Console output & input
#include <cstdlib>   // C STL
#include <cstdint>   // For uint32_t
#include <stdexcept> // For error gestion
#include <vector>    // To list variant types
#include <map>       // To classify elements in order
#include <optional>  // For the queue family indices
#include <set>       // To order the unique queue families
#include <limits>    // To use numeric_limits
#include <algorithm> // To use std::clamp
#include <fstream>   // To load shaders
#include <cstring>   // To use strcmp()

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() const {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class Vulkan3DEngine {
public:
	void run();

private:
	// Window initialization
	void initWindow();

	// Vulkan initialization
	void initVulkan();
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffer();
	void createSyncObjects();

	// Validation layers
	bool checkValidationLayerSupport();

	// Choosing devices
	int rateDeviceSuitability(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();

	// Shaders
	static std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);

	// Command buffer specific functions
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	// Debug functionnalities
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	// Main loop
	void mainLoop();
	void drawFrame();

	// Clean up
	void cleanup();
	
private:
	GLFWwindow* window;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	// Queues
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	// Swap chain
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;

	// Graphics pipeline
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	// Framebuffer
	std::vector<VkFramebuffer> swapChainFramebuffers;

	// Command pool -> drawing commands
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;

	// Semaphores
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;
};