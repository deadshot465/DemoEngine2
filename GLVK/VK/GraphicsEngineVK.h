#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "UtilsVK.h"

namespace GLVK
{
	namespace VK
	{
		class GraphicsEngine
		{
		public:
			GraphicsEngine(GLFWwindow* window, int width, int height);
			~GraphicsEngine();

			void Update(float deltaTime);
			void Render();
		
		private:
			static std::vector<const char*> GetRequiredExtensions(bool debug) noexcept;
			static bool CheckLayerSupport() noexcept;
			static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
			static bool IsDeviceSuitable(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) noexcept;
			static QueueIndices GetQueueIndices(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) noexcept;
			static bool CheckExtensionSupport(const vk::PhysicalDevice& device) noexcept;

			void Dispose();
			void CreateInstance();
			void SetupDebug();
			void CreateSurface();
			void GetPhysicalDevice();
			void CreateLogicalDevice();

			inline static const std::vector<const char*> m_enabledLayerNames = {
				"VK_LAYER_KHRONOS_validation"
			};
			inline static const std::vector<const char*> m_enabledExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

			GLFWwindow* m_handle = nullptr;
			int m_width = 0;
			int m_height = 0;
			bool m_debug = true;
			vk::Instance m_instance = nullptr;
			vk::DebugUtilsMessengerEXT m_debugUtils = nullptr;
			vk::PhysicalDevice m_physicalDevice = nullptr;
			vk::SurfaceKHR m_surface = nullptr;
			QueueIndices m_queueIndices = {};
			vk::Device m_logicalDevice = nullptr;
			vk::SwapchainKHR m_swapchain = nullptr;
			vk::Queue m_graphicsQueue = nullptr;
			vk::Queue m_presentQueue = nullptr;
		};
	}
}

