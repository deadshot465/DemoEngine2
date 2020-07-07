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
		struct QueueIndices;

		class GraphicsEngine
		{
		public:
			GraphicsEngine(GLFWwindow* window, int width, int height);
			~GraphicsEngine();

			void Update(float deltaTime);
			void Render();
		
		private:
			static std::vector<const char*> GetRequiredExtensions(bool debug);
			static bool CheckLayerSupport();
			static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
			static bool IsDeviceSuitable(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);
			static QueueIndices GetQueueIndices(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);
			static GLVK::VK::SwapchainDetails GetSwapchainDetails(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);
			static vk::Extent2D GetExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* handle) noexcept;
			static vk::SurfaceFormatKHR GetFormat(const std::vector<vk::SurfaceFormatKHR>& formats) noexcept;
			static vk::PresentModeKHR GetPresentMode(const std::vector<vk::PresentModeKHR>& presentModes) noexcept;

			void Dispose();
			void CreateInstance();
			void SetupDebug();
			void CreateSurface();
			void GetPhysicalDevice();
			void CreateSwapchain();

			inline static const std::vector<const char*> m_enabledLayerNames = {
				"VK_LAYER_KHRONOS_validation"
			};

			GLFWwindow* m_handle = nullptr;
			int m_width = 0;
			int m_height = 0;
			bool m_debug = true;
			vk::Instance m_instance = nullptr;
			vk::DebugUtilsMessengerEXT m_debugUtils = nullptr;
			vk::PhysicalDevice m_physicalDevice = nullptr;
			vk::SurfaceKHR m_surface = nullptr;
			vk::Device m_logicalDevice = nullptr;
			vk::SwapchainKHR m_swapchain = nullptr;
			vk::Queue m_graphicsQueue = nullptr;
			vk::Queue m_presentQueue = nullptr;
		};
	}
}

