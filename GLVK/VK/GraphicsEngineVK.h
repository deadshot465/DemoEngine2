#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "UtilsVK.h"

namespace GLVK
{
	namespace VK
	{
		class Buffer;
		class Image;
		class Shader;

		class GraphicsEngine
		{
		public:
			GraphicsEngine(GLFWwindow* window, int width, int height);
			~GraphicsEngine();

			void Update(float deltaTime);
			void Render();
		
		private:
			std::vector<Vertex> m_cubeVertices;
			std::vector<uint32_t> m_cubeIndices;

			static std::vector<const char*> GetRequiredExtensions(bool debug) noexcept;
			static bool CheckLayerSupport() noexcept;
			static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
			static bool IsDeviceSuitable(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) noexcept;
			static QueueIndices GetQueueIndices(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) noexcept;
			static bool CheckExtensionSupport(const vk::PhysicalDevice& device) noexcept;
			static GLVK::VK::SwapchainDetails GetSwapchainDetails(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);
			static vk::Extent2D GetExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* handle) noexcept;
			static vk::SurfaceFormatKHR GetFormat(const std::vector<vk::SurfaceFormatKHR>& formats) noexcept;
			static vk::PresentModeKHR GetPresentMode(const std::vector<vk::PresentModeKHR>& presentModes) noexcept;

			void Dispose();
			void CreateInstance();
			void SetupDebug();
			void CreateSurface();
			void GetPhysicalDevice();
			void CreateLogicalDevice();
			void CreateSwapchain();
			void LoadShader();
			void LoadDefaultCube();
			void CreateBuffers();
			void CreateVertexBuffers();
			void CreateIndexBuffers();

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
			vk::Format m_format = {};
			vk::Extent2D m_extent = {};
			vk::Queue m_graphicsQueue = nullptr;
			vk::Queue m_presentQueue = nullptr;

			std::vector<std::unique_ptr<Image>> m_images;
			std::unique_ptr<Shader> m_vertexShader;
			std::unique_ptr<Shader> m_fragmentShader;
			std::unique_ptr<Buffer> m_vertexBuffer;
			std::unique_ptr<Buffer> m_intermediateBuffer;
			std::unique_ptr<Buffer> m_indexBuffer;
		};
	}
}

