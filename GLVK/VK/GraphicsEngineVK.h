#pragma once
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <memory>
#include <string_view>
#include <vector>
#include "../../Interfaces/IGraphics.h"
#include "../../Structures/Model.h"
#include "../../Structures/Vertex.h"
#include "../../UtilsCommon.h"
#include "BufferVK.h"
#include "ImageVK.h"
#include "PipelineVK.h"
#include "ShaderVK.h"
#include "UtilsVK.h"

namespace GLVK
{
	namespace VK
	{
		using MESH = Mesh<Image, Buffer>;
		using MODEL = Model<Image, Buffer>;

		class Buffer;
		class Image;
		class Shader;
		class Pipeline;

		class GraphicsEngine
			: public IGraphics
		{
		public:
			GraphicsEngine(GLFWwindow* window, int width, int height, IResourceManager* resourceManager);
			~GraphicsEngine();

			virtual void Initialize() override;
			virtual void Update(float deltaTime) override;
			virtual void Render() override;
			virtual void BeginDraw() override;
			virtual void EndDraw() override;

			virtual std::shared_ptr<IDisposable> CreateVertexBuffer(const std::vector<Vertex>& vertices) override;
			virtual std::shared_ptr<IDisposable> CreateIndexBuffer(const std::vector<uint32_t>& indices) override;
			virtual std::tuple<IDisposable*, unsigned int> LoadTexture(std::string_view fileName) override;
			virtual std::tuple<IDisposable*, unsigned int> LoadModel(std::string_view modelName, const Vector3& position, const Vector3& scale, const Vector3& rotation, const Vector4& color) override;
			virtual std::tuple<IDisposable*, unsigned int> CreateMesh(const PrimitiveType& primitiveType, const Vector3& position, const Vector3& scale, const Vector3& rotation, const Vector4& color) override;
			
			const std::vector<vk::CommandBuffer>& GetCommandBufferOrLists() noexcept
			{
				return m_commandBuffers;
			}

			vk::DeviceSize GetDynamicOffset() const noexcept
			{
				return m_dynamicBufferObject.DynamicAlignment;
			}

			const Pipeline* GetPipeline() const noexcept
			{
				return m_pipeline.get();
			}

			const vk::DescriptorSet& GetDescriptorSet() const noexcept
			{
				return m_descriptorSet;
			}

			PushConstant& GetPushConstant() noexcept
			{
				return m_pushConstant;
			}

		private:
			inline static constexpr size_t DESCRIPTOR_TYPE_COUNT = 4;

			static std::vector<const char*> GetRequiredExtensions(bool debug) noexcept;
			static bool CheckLayerSupport() noexcept;
			static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
			static bool IsDeviceSuitable(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) noexcept;
			static QueueIndices GetQueueIndices(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) noexcept;
			static bool CheckExtensionSupport(const vk::PhysicalDevice& device) noexcept;
			static GLVK::VK::SwapchainDetails GetSwapchainDetails(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);
			static vk::Extent2D GetExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* handle) noexcept;
			static vk::SurfaceFormatKHR GetSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) noexcept;
			static vk::PresentModeKHR GetPresentMode(const std::vector<vk::PresentModeKHR>& presentModes) noexcept;
			static vk::Format ChooseDepthFormat(const vk::PhysicalDevice& physicalDevice, const std::vector<vk::Format>& formats, const vk::ImageTiling& imageTiling, const vk::FormatFeatureFlags& formatFeatures) noexcept;
			static vk::Format GetDepthFormat(const vk::PhysicalDevice& physicalDevice, const vk::ImageTiling& imageTiling) noexcept;
			static vk::SampleCountFlagBits GetMsaaSampleCounts(const vk::PhysicalDevice& physicalDevice);

			void Dispose();
			void CreateInstance();
			void SetupDebug();
			void CreateSurface();
			void GetPhysicalDevice();
			void CreateLogicalDevice();
			void CreateSwapchain();
			void LoadShader();
			void CreateDescriptorLayout();
			void CreateDescriptorSets();
			void CreateDepthImage();
			void CreateMultisamplingImage();
			void CreateUniformBuffers();
			void CreateFramebuffers();
			void CreateCommandBuffers();
			void CreateSynchronizationObjects();

			inline static const std::vector<const char*> m_enabledLayerNames = {
				"VK_LAYER_KHRONOS_validation"
			};
			inline static const std::vector<const char*> m_enabledExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

			bool m_debug = true;
			size_t m_currentImageIndex = 0;
			vk::Instance m_instance = nullptr;
			vk::DebugUtilsMessengerEXT m_debugUtils = nullptr;
			vk::PhysicalDevice m_physicalDevice = nullptr;
			vk::PhysicalDeviceProperties m_physicalDeviceProperties;
			vk::PhysicalDeviceFeatures m_physicalDeviceFeatures;
			vk::SampleCountFlagBits m_msaaSampleCount = {};
			vk::SurfaceKHR m_surface = nullptr;
			QueueIndices m_queueIndices = {};
			vk::Device m_logicalDevice = nullptr;
			SwapchainDetails m_swapchainDetails = {};
			vk::SurfaceFormatKHR m_surfaceFormat = {};
			vk::Format m_format = {};
			vk::SwapchainKHR m_swapchain = nullptr;
			
			vk::Extent2D m_extent = {};
			vk::Queue m_graphicsQueue = nullptr;
			vk::Queue m_presentQueue = nullptr;
			vk::CommandPool m_commandPool = nullptr;
			vk::DescriptorSetLayout m_descriptorSetLayout = nullptr;
			vk::DescriptorPool m_descriptorPool = nullptr;
			vk::DescriptorSet m_descriptorSet = nullptr;
			std::vector<vk::Framebuffer> m_framebuffers;
			std::vector<vk::CommandBuffer> m_commandBuffers;
			std::vector<vk::Semaphore> m_imageAcquiredSemaphores;
			std::vector<vk::Semaphore> m_renderCompletedSemaphores;
			std::vector<vk::Fence> m_fences;

			std::vector<std::unique_ptr<Image>> m_images;
			std::unique_ptr<Shader> m_vertexShader = nullptr;
			std::unique_ptr<Shader> m_fragmentShader = nullptr;
			std::unique_ptr<Shader> m_vertexShaderMesh = nullptr;
			std::unique_ptr<Buffer> m_intermediateBuffer = nullptr;
			//std::vector<std::unique_ptr<Buffer>> m_mvpBuffers;
			std::unique_ptr<Buffer> m_mvpBuffer = nullptr;
			//std::vector<std::unique_ptr<Buffer>> m_directionalLightBuffers;
			std::unique_ptr<Buffer> m_directionalLightBuffer = nullptr;
			std::unique_ptr<Buffer> m_dynamicMeshUniformBuffer = nullptr;
			std::unique_ptr<Buffer> m_dynamicModelUniformBuffer = nullptr;
			std::unique_ptr<Image> m_depthImage = nullptr;
			std::unique_ptr<Image> m_msaaImage = nullptr;
			std::unique_ptr<Pipeline> m_pipeline = nullptr;
			
			std::vector<Image*> m_textures;
			std::vector<MODEL*> m_models;
			std::vector<MESH*> m_meshes;

			MVP m_mvp = {};
			DirectionalLight m_directionalLight = {};
			PushConstant m_pushConstant = {};
			struct
			{
				DynamicBufferModels Models;
				DynamicBufferModels Meshes;
				vk::DeviceSize MinAlignment;
				vk::DeviceSize DynamicAlignment;
			} m_dynamicBufferObject;
		};
	}
}

