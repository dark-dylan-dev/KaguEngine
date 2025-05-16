#pragma once

// GLFW / Vulkan
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
#include <array>     // To describe the attributes of a vertex
#include <chrono>    // To keep precise timing when updating uniform buffers

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

namespace KaguEngine {

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		[[nodiscard]] bool isComplete() const {
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

	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}
	};

	const std::vector<Vertex> vertices = {
		// Syntax : {{posX, posY}, {R, G, B}}
		// ----------------------------------
		// Two combined triangles for a rectangle
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f,  -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f,  0.5f},  {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f},  {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
	};

	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	constexpr uint32_t WIDTH = 800;
	constexpr uint32_t HEIGHT = 600;

	constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	class App {
	public:
		void run();

	private:
		// Window initialization
		void initWindow();
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

		// Vulkan initialization - VulkanInit.cpp
		void initVulkan();
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createDescriptorSetLayout();
		void createGraphicsPipeline();
		void createFramebuffers();
		void createCommandPool();
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void createVertexBuffer();
		void createIndexBuffer();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();
		void createCommandBuffers();
		void createSyncObjects();

		// Validation layers - VulkanValidationLayers.cpp
		static bool checkValidationLayerSupport();

		// Choosing devices - VulkanDeviceChoice.cpp
		int rateDeviceSuitability(VkPhysicalDevice device) const;
		static std::vector<const char*> getRequiredExtensions();
		static bool checkDeviceExtensionSupport(VkPhysicalDevice device);

		// Swap chain - VulkanSwapChain.cpp
		void recreateSwapChain();
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;
		static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		[[nodiscard]] VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

		// Queues - VulkanQueues.cpp
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

		// Textures - VulkanTextureGestion.cpp
		void createTextureImage();
		VkImageView createImageView(VkImage image, VkFormat format) const;
		void createTextureImageView();
		void createTextureSampler();
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) const;
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;

		// Shaders - VulkanShaders.cpp
		static std::vector<char> readFile(const std::string& filename);
		[[nodiscard]] VkShaderModule createShaderModule(const std::vector<char>& code) const;

		// Command buffer & Frame rendering - VulkanFrameRendering.cpp
		[[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const;
		void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;
		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) const;
		void drawFrame();
		void updateUniformBuffer(uint32_t currentImage) const;

		// Vertex buffer construction
		[[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

		// Debug functionalities - VulkanDebugger.cpp
		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
		static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

		// Main loop - KaguEngine.cpp
		void mainLoop();

		// Clean up - VulkanCleanup.cpp
		void cleanupSwapChain() const;
		void cleanup() const;

	private:
		// GLFW window
		GLFWwindow* window;

		// Vulkan viewport
		bool framebufferResized = false;

		// Vulkan debugger
		VkDebugUtilsMessengerEXT debugMessenger;

		// Vulkan instance -> device -> surface
		VkInstance instance;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device;
		VkSurfaceKHR surface;

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

		// Descriptor sets
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;

		// Framebuffer
		std::vector<VkFramebuffer> swapChainFramebuffers;

		// Command pool -> drawing commands
		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

		// Vertex buffers
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		// Index buffers
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		// Uniform buffers
		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;

		// Semaphores
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

		// Textures image
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;

		// Index to count frames in flight
		uint32_t currentFrame = 0;
	}; // Class App

} // KaguEngine::