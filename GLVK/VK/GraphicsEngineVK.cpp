#include "GraphicsEngineVK.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <unordered_set>
#include <cassert>
#include <cmath>
#include <cstring>

#if defined(max)
#undef max
#endif
#if defined(min)
#undef min
#endif

GLVK::VK::GraphicsEngine::GraphicsEngine(GLFWwindow* window, int width, int height, IResourceManager* resourceManager)
	: IGraphics(window, width, height, resourceManager)
{
	try
	{
		CreateInstance();
		SetupDebug();
		CreateSurface();
		GetPhysicalDevice();
		CreateLogicalDevice();
		LoadShader();

		auto pool_info = vk::CommandPoolCreateInfo();
		pool_info.queueFamilyIndex = m_queueIndices.GraphicsQueue.value();
		m_commandPool = m_logicalDevice.createCommandPool(pool_info);

		m_swapchainDetails = GetSwapchainDetails(m_physicalDevice, m_surface);
		m_surfaceFormat = GetSurfaceFormat(m_swapchainDetails.Formats);
		m_extent = GetExtent(m_swapchainDetails.SurfaceCapabilities, reinterpret_cast<GLFWwindow*>(m_handle));
		m_format = m_surfaceFormat.format;
	}
	catch (const std::exception&)
	{
		throw;
	}
}

GLVK::VK::GraphicsEngine::~GraphicsEngine()
{
	Dispose();
	ReleaseAlignedMemory(m_dynamicBufferObject.Object.Buffer);
	for (auto& mesh : m_meshes)
		mesh.reset();
	m_logicalDevice.destroyDescriptorSetLayout(m_descriptorSetLayout);
	m_logicalDevice.destroyCommandPool(m_commandPool);
	if (m_intermediateBuffer) m_intermediateBuffer.reset();
	m_vertexShader.reset();
	m_fragmentShader.reset();
	m_logicalDevice.destroy();
	m_instance.destroySurfaceKHR(m_surface);
	auto dispatcher = vk::DispatchLoaderDynamic();
	dispatcher.init(m_instance, vkGetInstanceProcAddr);
	m_instance.destroyDebugUtilsMessengerEXT(m_debugUtils, nullptr, dispatcher);
	m_instance.destroy();
}

void GLVK::VK::GraphicsEngine::Update(float deltaTime)
{
    using namespace std::chrono;
    static auto start_time = high_resolution_clock::now();
    auto current_time = high_resolution_clock::now();
    auto duration_between = duration<float, seconds::period>(current_time - start_time).count();

    static auto elapsed_time_since_last_frame = 0.0f;
    elapsed_time_since_last_frame += deltaTime;

    auto rotate_x = glm::rotate(glm::mat4(1.0f), duration_between * glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    auto rotate_y = glm::rotate(glm::mat4(1.0f), duration_between * glm::radians(-45.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    auto rotate_z = glm::rotate(glm::mat4(1.0f), duration_between * glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    m_mvp.Model = rotate_z * rotate_y * rotate_x * glm::mat4(1.0f);
	auto mapped = m_mvpBuffer->Map(sizeof(MVP));
    memcpy(mapped, &m_mvp, sizeof(MVP));

	for (auto i = 0; i < m_dynamicBufferObject.Object.Models.size(); ++i)
	{
		auto& model = m_models[i];
		model->RotationX += glm::radians(duration_between / 500.0f);
		model->RotationY += glm::radians(duration_between / 500.0f);
		model->RotationZ += glm::radians(duration_between / 500.0f);
		auto& world = m_dynamicBufferObject.Object.Models[m_dynamicBufferObject.Object.ModelIndices[i]];
		world = model->GetWorldMatrix();

		auto ptr = reinterpret_cast<glm::mat4*>(reinterpret_cast<uint64_t>(m_dynamicBufferObject.Object.Buffer) + (m_dynamicBufferObject.DynamicAlignment * i));

		*ptr = world;
	}

	mapped = m_dynamicUniformBuffer->Map(VK_WHOLE_SIZE);
	memcpy(mapped, m_dynamicBufferObject.Object.Buffer, m_dynamicUniformBuffer->GetBufferSize());
}

void GLVK::VK::GraphicsEngine::Render()
{
    auto result = m_logicalDevice.acquireNextImageKHR(m_swapchain, std::numeric_limits<uint32_t>::max(), m_imageAcquiredSemaphores[m_currentImageIndex], nullptr);

    vk::PipelineStageFlags wait_stages[] = {
            vk::PipelineStageFlagBits::eColorAttachmentOutput
    };

    auto submit_info = vk::SubmitInfo();
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &m_commandBuffers[m_currentImageIndex];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &m_renderCompletedSemaphores[m_currentImageIndex];
    submit_info.pWaitSemaphores = &m_imageAcquiredSemaphores[m_currentImageIndex];
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.waitSemaphoreCount = 1;

    m_logicalDevice.resetFences(m_fences[m_currentImageIndex]);
    m_graphicsQueue.submit(submit_info, m_fences[m_currentImageIndex]);

    auto present_info = vk::PresentInfoKHR();
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &m_renderCompletedSemaphores[m_currentImageIndex];
    present_info.pImageIndices = &result.value;
    present_info.pResults = nullptr;
    present_info.pSwapchains = &m_swapchain;
    present_info.swapchainCount = 1;

    m_presentQueue.presentKHR(present_info);
    m_logicalDevice.waitForFences(m_fences[m_currentImageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
    m_currentImageIndex = (m_currentImageIndex + 1) % m_images.size();
}

void GLVK::VK::GraphicsEngine::BeginDraw()
{
	static const auto clear_color = vk::ClearColorValue(std::array<float, 4>{ 1.0f, 1.0f, 1.0f, 1.0f });
	static const auto clear_depth = vk::ClearDepthStencilValue(1.0f, 0);
	static const vk::ClearValue clear_values[] = { vk::ClearValue(clear_color), vk::ClearValue(clear_depth) };
	auto renderpass_info = vk::RenderPassBeginInfo();
	renderpass_info.renderPass = m_pipeline->GetRenderPass();
	renderpass_info.renderArea.extent = m_extent;
	renderpass_info.renderArea.offset = vk::Offset2D();
	renderpass_info.clearValueCount = _countof(clear_values);
	renderpass_info.pClearValues = clear_values;
	auto begin_info = vk::CommandBufferBeginInfo();
	begin_info.pInheritanceInfo = nullptr;

	auto scissor = vk::Rect2D();
	scissor.extent = m_extent;

	for (auto i = 0; i < m_commandBuffers.size(); ++i)
	{
		renderpass_info.framebuffer = m_framebuffers[i];
		m_commandBuffers[i].begin(begin_info);
		m_commandBuffers[i].beginRenderPass(renderpass_info, vk::SubpassContents::eInline);

		m_commandBuffers[i].setViewport(0, { vk::Viewport(0.0f, 0.0f, static_cast<float>(m_extent.width), static_cast<float>(m_extent.height), 0.0f, 1.0f) });
		m_commandBuffers[i].setScissor(0, { scissor });
		/*m_commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipeline->GetPipelineLayout(), 0, { m_descriptorSet }, { 0 });*/
		m_commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->GetPipeline(BlendMode::None));
	}
}

void GLVK::VK::GraphicsEngine::EndDraw()
{
	for (auto i = 0; i < m_commandBuffers.size(); ++i)
	{
		m_commandBuffers[i].endRenderPass();
		m_commandBuffers[i].end();
	}
}

std::shared_ptr<IDisposable> GLVK::VK::GraphicsEngine::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
	vk::DeviceSize buffer_size = sizeof(Vertex) * vertices.size();
	m_intermediateBuffer.reset(new Buffer(m_logicalDevice, vk::BufferUsageFlagBits::eTransferSrc, buffer_size));
	auto memory = m_intermediateBuffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	void* mapped = m_logicalDevice.mapMemory(memory, 0, buffer_size);
	memcpy(mapped, vertices.data(), buffer_size);
	m_logicalDevice.unmapMemory(memory);

	auto buffer = std::make_shared<Buffer>(m_logicalDevice, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, buffer_size);
	buffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
	buffer->CopyBufferToBuffer(m_intermediateBuffer->GetBuffer(), buffer_size, m_commandPool, m_graphicsQueue);
	return buffer;
}

std::shared_ptr<IDisposable> GLVK::VK::GraphicsEngine::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
	vk::DeviceSize buffer_size = sizeof(uint32_t) * indices.size();
	m_intermediateBuffer.reset(new Buffer(m_logicalDevice, vk::BufferUsageFlagBits::eTransferSrc, buffer_size));
	auto memory = m_intermediateBuffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	void* mapped = m_logicalDevice.mapMemory(memory, 0, buffer_size);
	memcpy(mapped, indices.data(), buffer_size);
	m_logicalDevice.unmapMemory(memory);

	auto buffer = std::make_shared<Buffer>(m_logicalDevice, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, buffer_size);
	buffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
	buffer->CopyBufferToBuffer(m_intermediateBuffer->GetBuffer(), buffer_size, m_commandPool, m_graphicsQueue);
	return buffer;
}

std::tuple<IDisposable*, unsigned int> GLVK::VK::GraphicsEngine::LoadTexture(std::string_view fileName)
{
	int width = 0;
	int height = 0;
	int channels = 0;
	auto image = stbi_load(fileName.data(), &width, &height, &channels, STBI_rgb_alpha);
	auto size = static_cast<vk::DeviceSize>(width) * height * 4;

	m_intermediateBuffer.reset(new Buffer(m_logicalDevice, vk::BufferUsageFlagBits::eTransferSrc, size));
	m_intermediateBuffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	auto mapped_data = m_logicalDevice.mapMemory(m_intermediateBuffer->GetDeviceMemory(), 0, size);
	memcpy(mapped_data, image, size);
	m_logicalDevice.unmapMemory(m_intermediateBuffer->GetDeviceMemory());
	stbi_image_free(image);

	auto mip_level_count = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	auto texture = std::make_unique<Image>(m_logicalDevice, m_format, vk::SampleCountFlagBits::e1, vk::Extent2D(static_cast<uint32_t>(width), static_cast<uint32_t>(height)), vk::ImageType::e2D, mip_level_count, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
	texture->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
	texture->TransitionLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, m_commandPool, m_graphicsQueue, vk::ImageAspectFlagBits::eColor, mip_level_count);
	m_intermediateBuffer->CopyBufferToImage(texture->GetImage(), static_cast<uint32_t>(height), static_cast<uint32_t>(width), size, vk::ImageAspectFlagBits::eColor, m_commandPool, m_graphicsQueue);
	texture->GenerateMipmaps(m_commandPool, m_graphicsQueue, mip_level_count);
	texture->CreateImageView(m_format, vk::ImageAspectFlagBits::eColor, mip_level_count, vk::ImageViewType::e2D);
	texture->CreateSampler(mip_level_count);
	auto ptr = m_textures.emplace_back(m_resourceManager->AddResource(texture));
	auto index = m_textures.empty() ? 0 : m_textures.size() - 1;
	return std::make_tuple(ptr, static_cast<uint32_t>(index));
}

std::tuple<IDisposable*, unsigned int> GLVK::VK::GraphicsEngine::LoadModel(std::string_view modelName, const Vector3& position, const Vector3& scale, const Vector3& rotation, const Vector4& color)
{
	auto resource = m_resourceManager->GetResource<MODEL>(modelName);
	auto model = std::make_unique<MODEL>();
	if (resource)
	{
		model->Color = color;
		model->Meshes = resource->Meshes;
		model->Position = position;
		model->RotationX = glm::radians(rotation.x);
		model->RotationY = glm::radians(rotation.y);
		model->RotationZ = glm::radians(rotation.z);
		model->ScaleX = scale.x;
		model->ScaleY = scale.y;
		model->ScaleZ = scale.z;
	}
	else
	{
		model->Load(modelName, this, position, scale, rotation, color);	
	}
	auto ptr = m_models.emplace_back(m_resourceManager->AddResource(model, modelName));
	for (auto& mesh : ptr->Meshes)
	{
		mesh.VertexBuffer = std::dynamic_pointer_cast<Buffer>(CreateVertexBuffer(mesh.Vertices));
		mesh.IndexBuffer = std::dynamic_pointer_cast<Buffer>(CreateIndexBuffer(mesh.Indices));
	}
	size_t index = m_dynamicBufferObject.Object.ModelIndices.emplace_back(static_cast<uint32_t>(m_dynamicBufferObject.Object.ModelIndices.size()));
	m_dynamicBufferObject.Object.Models.emplace_back(ptr->GetWorldMatrix());
	return std::make_tuple(ptr, static_cast<uint32_t>(index));
}

std::vector<const char*> GLVK::VK::GraphicsEngine::GetRequiredExtensions(bool debug) noexcept
{
	uint32_t extension_count = 0;
	auto instance_extensions = glfwGetRequiredInstanceExtensions(&extension_count);
	auto extensions = std::vector<const char*>(instance_extensions, instance_extensions + extension_count);
	
	if (debug)
	{
		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return extensions;
}

bool GLVK::VK::GraphicsEngine::CheckLayerSupport() noexcept
{
	auto layer_properties = vk::enumerateInstanceLayerProperties();
	for (const auto& layer : m_enabledLayerNames)
	{
		for (const auto& property : layer_properties)
		{
			if (std::strcmp(property.layerName, layer) == 0)
			{
				return true;
			}
		}
	}
	return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GLVK::VK::GraphicsEngine::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "Debug Message: " << pCallbackData->pMessage << '\n';
	return VK_FALSE;
}

bool GLVK::VK::GraphicsEngine::IsDeviceSuitable(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) noexcept
{
	auto queue_indices = GetQueueIndices(device, surface);
	if (!queue_indices.IsCompleted())
		return false;
	
	auto properties = device.getProperties();
	std::cout << "Device: " << properties.deviceName << '\n';

	auto features = device.getFeatures();
	auto feature_supports = features.samplerAnisotropy && features.sampleRateShading && features.shaderSampledImageArrayDynamicIndexing;
	
	auto indexing_feature = vk::PhysicalDeviceDescriptorIndexingFeatures();
	auto feature2 = vk::PhysicalDeviceFeatures2();
	feature2.pNext = &indexing_feature;
	device.getFeatures2(&feature2);

	auto swapchain_details = GetSwapchainDetails(device, surface);
	auto is_swapchain_adequate = swapchain_details.Formats.size() > 0 && swapchain_details.PresentModes.size() > 0;

	return feature_supports &&
		CheckExtensionSupport(device) &&
		is_swapchain_adequate &&
		indexing_feature.descriptorBindingPartiallyBound &&
		indexing_feature.runtimeDescriptorArray;
}

GLVK::VK::QueueIndices GLVK::VK::GraphicsEngine::GetQueueIndices(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) noexcept
{
	auto queue_indices = QueueIndices();
	auto properties = device.getQueueFamilyProperties();
	
	for (uint32_t i = 0; i < properties.size(); ++i)
	{
		auto surface_support = device.getSurfaceSupportKHR(i, surface);

		if (properties[i].queueCount > 0 && (properties[i].queueFlags & vk::QueueFlagBits::eGraphics))
		{
			queue_indices.GraphicsQueue = i;
		}

		if (properties[i].queueCount > 0 && surface_support)
		{
			queue_indices.PresentQueue = i;
		}

		if (queue_indices.IsCompleted())
			return queue_indices;
	}
	return QueueIndices();
}

bool GLVK::VK::GraphicsEngine::CheckExtensionSupport(const vk::PhysicalDevice& device) noexcept
{
	auto properties = device.enumerateDeviceExtensionProperties();
	auto enabled_extensions = std::unordered_set<std::string>(m_enabledExtensions.cbegin(), m_enabledExtensions.cend());

	for (const auto& property : properties)
	{
		enabled_extensions.erase(property.extensionName.data());
	}

	return enabled_extensions.empty();
}

void GLVK::VK::GraphicsEngine::Initialize()
{
	try
	{
		CreateSwapchain();
		CreateUniformBuffers();
		CreateDescriptorLayout();
		CreateDescriptorSets();
		CreateDepthImage();
		CreateMultisamplingImage();
		m_pipeline = std::make_unique<Pipeline>(m_logicalDevice);
		m_pipeline->CreateRenderPass(m_format, GetDepthFormat(m_physicalDevice, vk::ImageTiling::eOptimal), m_msaaSampleCount);
		m_pipeline->CreateGraphicPipelines(m_descriptorSetLayout, m_msaaSampleCount, {
			m_vertexShader->GetShaderStageInfo(),
			m_fragmentShader->GetShaderStageInfo()
			});
		CreateFramebuffers();
		CreateCommandBuffers();
		CreateSynchronizationObjects();
		//BeginRenderPass();
	}
	catch (const std::exception&)
	{
		throw;
	}
}

void GLVK::VK::GraphicsEngine::Dispose()
{
    m_logicalDevice.waitIdle();
    m_logicalDevice.freeCommandBuffers(m_commandPool, m_commandBuffers);
	m_pipeline.reset();
	
	m_dynamicUniformBuffer.reset();
	m_mvpBuffer.reset();
	m_directionalLightBuffer.reset();

	for (auto i = 0; i < m_images.size(); ++i)
	{
	    m_logicalDevice.destroyFence(m_fences[i]);
	    m_logicalDevice.destroySemaphore(m_imageAcquiredSemaphores[i]);
	    m_logicalDevice.destroySemaphore(m_renderCompletedSemaphores[i]);
	    m_logicalDevice.destroyFramebuffer(m_framebuffers[i]);
		//m_mvpBuffers[i].reset();
		//m_directionalLightBuffers[i].reset();
	}

	m_msaaImage.reset();
	m_depthImage.reset();
	m_logicalDevice.destroyDescriptorPool(m_descriptorPool);

	for (auto& image : m_images)
	{
		image.reset();
	}
	m_logicalDevice.destroySwapchainKHR(m_swapchain);
}

void GLVK::VK::GraphicsEngine::CreateInstance()
{
	auto app_info = vk::ApplicationInfo();
	app_info.apiVersion = VK_API_VERSION_1_2;
	app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	app_info.pApplicationName = "Demo Application";
	app_info.pEngineName = "Demo Engine";
	
	auto instance_info = vk::InstanceCreateInfo();
	auto extensions = GetRequiredExtensions(m_debug);
	instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instance_info.pApplicationInfo = &app_info;
	instance_info.ppEnabledExtensionNames = extensions.data();
	
	if (m_debug)
	{
		instance_info.enabledLayerCount = static_cast<uint32_t>(m_enabledLayerNames.size());
		instance_info.ppEnabledLayerNames = m_enabledLayerNames.data();
	}

	m_instance = vk::createInstance(instance_info);
}

void GLVK::VK::GraphicsEngine::SetupDebug()
{
	if (m_debug && !CheckLayerSupport())
	{
		throw std::runtime_error("Debug layer required but not supported.\n");
	}

	auto info = vk::DebugUtilsMessengerCreateInfoEXT();
	info.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
	info.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
	info.pfnUserCallback = DebugCallback;
	info.pUserData = nullptr;
	
	auto dispatcher = vk::DispatchLoaderDynamic();
	dispatcher.init(m_instance, vkGetInstanceProcAddr);
	m_debugUtils = m_instance.createDebugUtilsMessengerEXT(info, nullptr, dispatcher);
}

void GLVK::VK::GraphicsEngine::CreateSurface()
{
	assert(m_instance);
	auto res = glfwCreateWindowSurface(m_instance, reinterpret_cast<GLFWwindow*>(m_handle), nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_surface));
	ThrowIfFailed(res, "Failed to create surface.\n");
}

void GLVK::VK::GraphicsEngine::GetPhysicalDevice()
{
	auto physical_devices = m_instance.enumeratePhysicalDevices();
	for (const auto& device : physical_devices)
	{
		if (IsDeviceSuitable(device, m_surface))
		{
			m_physicalDevice = device;
			m_msaaSampleCount = GetMsaaSampleCounts(device);
			m_physicalDeviceProperties = device.getProperties();
			m_physicalDeviceFeatures = device.getFeatures();
			break;
		}
	}
}

void GLVK::VK::GraphicsEngine::CreateLogicalDevice()
{
	m_queueIndices = GetQueueIndices(m_physicalDevice, m_surface);

	auto queue_create_infos = std::vector<vk::DeviceQueueCreateInfo>();
	auto unique_indices = std::unordered_set<uint32_t>{
		m_queueIndices.GraphicsQueue.value(),
		m_queueIndices.PresentQueue.value()
	};
	auto priority = 1.0f;
	for (const auto& index : unique_indices)
	{
		auto info = vk::DeviceQueueCreateInfo();
		info.pQueuePriorities = &priority;
		info.queueCount = 1;
		info.queueFamilyIndex = index;
		queue_create_infos.emplace_back(info);
	}

	auto features = vk::PhysicalDeviceFeatures();
	features.samplerAnisotropy = VK_TRUE;
	features.sampleRateShading = VK_TRUE;
	features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;

	auto indexing_features = vk::PhysicalDeviceDescriptorIndexingFeatures();
	indexing_features.descriptorBindingPartiallyBound = VK_TRUE;
	indexing_features.runtimeDescriptorArray = VK_TRUE;

	auto info = vk::DeviceCreateInfo();
	info.enabledExtensionCount = static_cast<uint32_t>(m_enabledExtensions.size());
	info.pEnabledFeatures = &features;
	info.ppEnabledExtensionNames = m_enabledExtensions.data();
	info.pQueueCreateInfos = queue_create_infos.data();
	info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
	info.pNext = &indexing_features;

	if (m_debug)
	{
		info.enabledLayerCount = static_cast<uint32_t>(m_enabledLayerNames.size());
		info.ppEnabledLayerNames = m_enabledLayerNames.data();
	}

	m_logicalDevice = m_physicalDevice.createDevice(info);
	m_graphicsQueue = m_logicalDevice.getQueue(0, m_queueIndices.GraphicsQueue.value());
	m_presentQueue = m_logicalDevice.getQueue(0, m_queueIndices.PresentQueue.value());
}

void GLVK::VK::GraphicsEngine::CreateSwapchain()
{
	auto present_mode = GetPresentMode(m_swapchainDetails.PresentModes);

	uint32_t min_image_count = 0;
	if (m_swapchainDetails.SurfaceCapabilities.maxImageCount > 0)
	{
		min_image_count = std::clamp<uint32_t>(m_swapchainDetails.SurfaceCapabilities.minImageCount + 1, m_swapchainDetails.SurfaceCapabilities.minImageCount, m_swapchainDetails.SurfaceCapabilities.maxImageCount);
	}
	else
	{
		min_image_count = m_swapchainDetails.SurfaceCapabilities.minImageCount + 1;
	}

	auto info = vk::SwapchainCreateInfoKHR();
	info.oldSwapchain = nullptr;
	info.clipped = VK_FALSE;
	info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	info.imageArrayLayers = 1;
	info.imageColorSpace = m_surfaceFormat.colorSpace;
	info.imageExtent = m_extent;
	info.imageFormat = m_surfaceFormat.format;
	info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	info.minImageCount = min_image_count;
	info.preTransform = m_swapchainDetails.SurfaceCapabilities.currentTransform;
	info.presentMode = present_mode;
	info.surface = m_surface;
	
	uint32_t indices[] = {
		m_queueIndices.GraphicsQueue.value(),
		m_queueIndices.PresentQueue.value()
	};

	if (indices[0] != indices[1])
	{
		info.imageSharingMode = vk::SharingMode::eConcurrent;
		info.queueFamilyIndexCount = 2;
		info.pQueueFamilyIndices = indices;
	}
	else
	{
		info.imageSharingMode = vk::SharingMode::eExclusive;
		info.queueFamilyIndexCount = 0;
		info.pQueueFamilyIndices = nullptr;
	}

	m_swapchain = m_logicalDevice.createSwapchainKHR(info);

	auto images = m_logicalDevice.getSwapchainImagesKHR(m_swapchain);
	m_images.resize(images.size());
	for (auto i = 0; i < images.size(); ++i)
	{
		m_images[i] = std::make_unique<Image>(m_logicalDevice, images[i]);
		m_images[i]->CreateImageView(m_format, vk::ImageAspectFlagBits::eColor, 1, vk::ImageViewType::e2D);
	}
}

void GLVK::VK::GraphicsEngine::LoadShader()
{
	m_vertexShader = std::make_unique<Shader>(vk::ShaderStageFlagBits::eVertex, "GLVK/VK/Shaders/vert.spv", m_logicalDevice);
	m_fragmentShader = std::make_unique<Shader>(vk::ShaderStageFlagBits::eFragment, "GLVK/VK/Shaders/frag.spv", m_logicalDevice);
}

void GLVK::VK::GraphicsEngine::CreateDescriptorLayout()
{
	vk::DescriptorSetLayoutBinding bindings[DESCRIPTOR_TYPE_COUNT] = {};
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
	bindings[0].pImmutableSamplers = nullptr;
	bindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex;
	
	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = vk::DescriptorType::eUniformBuffer;
	bindings[1].pImmutableSamplers = nullptr;
	bindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;

	bindings[2].binding = 2;
	bindings[2].descriptorCount = 1;
	bindings[2].descriptorType = vk::DescriptorType::eUniformBufferDynamic;
	bindings[2].pImmutableSamplers = nullptr;
	bindings[2].stageFlags = vk::ShaderStageFlagBits::eVertex;

	/*bindings[2].binding = 3;
	bindings[2].descriptorCount = static_cast<uint32_t>(m_textures.size());
	bindings[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	bindings[2].pImmutableSamplers = nullptr;
	bindings[2].stageFlags = vk::ShaderStageFlagBits::eFragment;*/

	auto info = vk::DescriptorSetLayoutCreateInfo();
	info.bindingCount = static_cast<uint32_t>(_countof(bindings));
	info.pBindings = bindings;
	m_descriptorSetLayout = m_logicalDevice.createDescriptorSetLayout(info);
}

void GLVK::VK::GraphicsEngine::CreateDescriptorSets()
{
	vk::DescriptorPoolSize pool_sizes[DESCRIPTOR_TYPE_COUNT] = {};
	pool_sizes[0].descriptorCount = 1;
	pool_sizes[0].type = vk::DescriptorType::eUniformBuffer;
	pool_sizes[1].descriptorCount = 1;
	pool_sizes[1].type = vk::DescriptorType::eUniformBuffer;;
	pool_sizes[2].descriptorCount = 1;
	pool_sizes[2].type = vk::DescriptorType::eUniformBufferDynamic;
	/*pool_sizes[2].descriptorCount = m_textures.empty() ? 1 : static_cast<uint32_t>(m_textures.size());
	pool_sizes[2].type = vk::DescriptorType::eCombinedImageSampler;*/

	assert(!m_images.empty());
	auto pool_info = vk::DescriptorPoolCreateInfo();
	pool_info.maxSets = static_cast<uint32_t>(m_images.size());
	pool_info.poolSizeCount = _countof(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	m_descriptorPool = m_logicalDevice.createDescriptorPool(pool_info);

	auto set_layouts = std::vector<vk::DescriptorSetLayout>(m_images.size(), m_descriptorSetLayout);
	auto allocate_info = vk::DescriptorSetAllocateInfo();
	allocate_info.descriptorPool = m_descriptorPool;
	allocate_info.descriptorSetCount = 1;
	allocate_info.pSetLayouts = &m_descriptorSetLayout;
	//allocate_info.descriptorSetCount = static_cast<uint32_t>(set_layouts.size());
	//allocate_info.pSetLayouts = set_layouts.data();
	m_descriptorSet = m_logicalDevice.allocateDescriptorSets(allocate_info)[0];

	/*auto images_infos = std::vector<vk::DescriptorImageInfo>(m_textures.size());
	for (auto i = 0; i < m_textures.size(); ++i)
	{
		images_infos[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		images_infos[i].imageView = m_textures[i]->GetImageView();
		images_infos[i].sampler = m_textures[i]->GetSampler();
	}*/

	auto mvp_buffer_info = vk::DescriptorBufferInfo();
	mvp_buffer_info.buffer = m_mvpBuffer->GetBuffer();
	mvp_buffer_info.offset = 0;
	mvp_buffer_info.range = sizeof(MVP);

	auto directional_light_buffer_info = vk::DescriptorBufferInfo();
	directional_light_buffer_info.buffer = m_directionalLightBuffer->GetBuffer();
	directional_light_buffer_info.offset = 0;
	directional_light_buffer_info.range = sizeof(DirectionalLight);

	auto dynamic_buffer_info = vk::DescriptorBufferInfo();
	dynamic_buffer_info.buffer = m_dynamicUniformBuffer->GetBuffer();
	dynamic_buffer_info.offset = 0;
	dynamic_buffer_info.range = VK_WHOLE_SIZE;

	auto write_descriptor_count = DESCRIPTOR_TYPE_COUNT;
	auto write_descriptors = std::vector<vk::WriteDescriptorSet>(write_descriptor_count);
	write_descriptors[0].descriptorCount = 1;
	write_descriptors[0].descriptorType = vk::DescriptorType::eUniformBuffer;
	write_descriptors[0].dstArrayElement = 0;
	write_descriptors[0].dstBinding = 0;
	write_descriptors[0].dstSet = m_descriptorSet;
	write_descriptors[0].pBufferInfo = &mvp_buffer_info;
	write_descriptors[0].pImageInfo = nullptr;
	write_descriptors[0].pTexelBufferView = nullptr;

	write_descriptors[1].descriptorCount = 1;
	write_descriptors[1].descriptorType = vk::DescriptorType::eUniformBuffer;
	write_descriptors[1].dstArrayElement = 0;
	write_descriptors[1].dstBinding = 1;
	write_descriptors[1].dstSet = m_descriptorSet;
	write_descriptors[1].pBufferInfo = &directional_light_buffer_info;
	write_descriptors[1].pImageInfo = nullptr;
	write_descriptors[1].pTexelBufferView = nullptr;

	write_descriptors[2].descriptorCount = 1;
	write_descriptors[2].descriptorType = vk::DescriptorType::eUniformBufferDynamic;
	write_descriptors[2].dstArrayElement = 0;
	write_descriptors[2].dstBinding = 2;
	write_descriptors[2].dstSet = m_descriptorSet;
	write_descriptors[2].pBufferInfo = &dynamic_buffer_info;
	write_descriptors[2].pImageInfo = nullptr;
	write_descriptors[2].pTexelBufferView = nullptr;

	m_logicalDevice.updateDescriptorSets(write_descriptors, {});

	//for (auto i = 0; i < m_descriptorSets.size(); ++i)
	//{
	//	auto mvp_buffer_info = vk::DescriptorBufferInfo();
	//	mvp_buffer_info.buffer = m_mvpBuffer->GetBuffer();
	//	mvp_buffer_info.offset = 0;
	//	mvp_buffer_info.range = sizeof(MVP);
	//	
	//	auto directional_light_buffer_info = vk::DescriptorBufferInfo();
	//	directional_light_buffer_info.buffer = m_directionalLightBuffer->GetBuffer();
	//	directional_light_buffer_info.offset = 0;
	//	directional_light_buffer_info.range = sizeof(DirectionalLight);

	//	/*auto dynamic_buffer_info = vk::DescriptorBufferInfo();
	//	dynamic_buffer_info.buffer = m_dynamicUniformBuffer->GetBuffer();
	//	dynamic_buffer_info.offset = 0;
	//	dynamic_buffer_info.range = VK_WHOLE_SIZE;*/

	//	//auto write_descriptor_count = 2 + (m_textures.empty() ? 0 : 1);
	//	auto write_descriptor_count = 2;
	//	auto write_descriptors = std::vector<vk::WriteDescriptorSet>(write_descriptor_count);
	//	write_descriptors[0].descriptorCount = 1;
	//	write_descriptors[0].descriptorType = vk::DescriptorType::eUniformBuffer;
	//	write_descriptors[0].dstArrayElement = 0;
	//	write_descriptors[0].dstBinding = 0;
	//	write_descriptors[0].dstSet = m_descriptorSets[i];
	//	write_descriptors[0].pBufferInfo = &mvp_buffer_info;
	//	write_descriptors[0].pImageInfo = nullptr;
	//	write_descriptors[0].pTexelBufferView = nullptr;
	//	
	//	write_descriptors[1].descriptorCount = 1;
	//	write_descriptors[1].descriptorType = vk::DescriptorType::eUniformBuffer;
	//	write_descriptors[1].dstArrayElement = 0;
	//	write_descriptors[1].dstBinding = 1;
	//	write_descriptors[1].dstSet = m_descriptorSets[i];
	//	write_descriptors[1].pBufferInfo = &directional_light_buffer_info;
	//	write_descriptors[1].pImageInfo = nullptr;
	//	write_descriptors[1].pTexelBufferView = nullptr;

	//	/*write_descriptors[2].descriptorCount = 1;
	//	write_descriptors[2].descriptorType = vk::DescriptorType::eUniformBufferDynamic;
	//	write_descriptors[2].dstArrayElement = 0;
	//	write_descriptors[2].dstBinding = 2;
	//	write_descriptors[2].dstSet = m_descriptorSets[i];
	//	write_descriptors[2].pBufferInfo = &dynamic_buffer_info;
	//	write_descriptors[2].pImageInfo = nullptr;
	//	write_descriptors[2].pTexelBufferView = nullptr;*/

	//	/*if (!m_textures.empty())
	//	{
	//		write_descriptors[2].descriptorCount = static_cast<uint32_t>(m_textures.size()) <= 0 ? 1 : static_cast<uint32_t>(m_textures.size());
	//		write_descriptors[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	//		write_descriptors[2].dstArrayElement = 0;
	//		write_descriptors[2].dstBinding = 2;
	//		write_descriptors[2].dstSet = m_descriptorSets[i];
	//		write_descriptors[2].pBufferInfo = nullptr;
	//		write_descriptors[2].pImageInfo = images_infos.data();
	//		write_descriptors[2].pTexelBufferView = nullptr;
	//	}*/

	//	m_logicalDevice.updateDescriptorSets(write_descriptors, {});
	//}
}

void GLVK::VK::GraphicsEngine::CreateDepthImage()
{
	auto format = GetDepthFormat(m_physicalDevice, vk::ImageTiling::eOptimal);
	m_depthImage = std::make_unique<Image>(m_logicalDevice, format, m_msaaSampleCount, m_extent, vk::ImageType::e2D, 1, vk::ImageUsageFlagBits::eDepthStencilAttachment);
	m_depthImage->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
	m_depthImage->CreateImageView(format, vk::ImageAspectFlagBits::eDepth, 1, vk::ImageViewType::e2D);
	m_depthImage->TransitionLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, m_commandPool, m_graphicsQueue, vk::ImageAspectFlagBits::eDepth, 1);
}

void GLVK::VK::GraphicsEngine::CreateMultisamplingImage()
{
	m_msaaImage = std::make_unique<Image>(m_logicalDevice, m_format, m_msaaSampleCount, m_extent, vk::ImageType::e2D, 1, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment);
	m_msaaImage->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
	m_msaaImage->CreateImageView(m_format, vk::ImageAspectFlagBits::eColor, 1, vk::ImageViewType::e2D);
	m_msaaImage->TransitionLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, m_commandPool, m_graphicsQueue, vk::ImageAspectFlagBits::eColor, 1);
}

void GLVK::VK::GraphicsEngine::CreateUniformBuffers()
{
	vk::DeviceSize mvp_size = sizeof(MVP);
	vk::DeviceSize directional_light_size = sizeof(DirectionalLight);

	m_mvp.Model = glm::mat4(1.0f);
	m_mvp.View = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	m_mvp.Projection = glm::perspective(glm::radians(45.0f), static_cast<float>(m_width) / static_cast<float>(m_height), 0.1f, 100.0f);

	/*m_mvpBuffers.resize(m_images.size());
	for (auto& buffer : m_mvpBuffers)
	{
		buffer = std::make_unique<Buffer>(m_logicalDevice, vk::BufferUsageFlagBits::eUniformBuffer, mvp_size);
		auto mvp_memory = buffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		auto mapped = m_logicalDevice.mapMemory(mvp_memory, 0, mvp_size);
		memcpy(mapped, &m_mvp, mvp_size);
		m_logicalDevice.unmapMemory(mvp_memory);
	}*/

	m_mvpBuffer = std::make_unique<Buffer>(m_logicalDevice, vk::BufferUsageFlagBits::eUniformBuffer, mvp_size);
	auto mvp_memory = m_mvpBuffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	auto mapped = m_mvpBuffer->Map(mvp_size);
	memcpy(mapped, &m_mvp, mvp_size);
	
	m_directionalLight.AmbientIntensity = 0.1f;
	m_directionalLight.SpecularIntensity = 0.5f;
	m_directionalLight.Diffuse = glm::vec4(1.0f);
	m_directionalLight.LightDirection = glm::vec3(0.0f, -5.0f, 0.0f);
	
	/*m_directionalLightBuffers.resize(m_images.size());
	for (auto& buffer : m_directionalLightBuffers)
	{
		buffer = std::make_unique<Buffer>(m_logicalDevice, vk::BufferUsageFlagBits::eUniformBuffer, directional_light_size);
		auto directional_light_memory = buffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		auto mapped = m_logicalDevice.mapMemory(directional_light_memory, 0, directional_light_size);
		memcpy(mapped, &m_directionalLight, directional_light_size);
		m_logicalDevice.unmapMemory(directional_light_memory);
	}*/

	m_directionalLightBuffer = std::make_unique<Buffer>(m_logicalDevice, vk::BufferUsageFlagBits::eUniformBuffer, directional_light_size);
	auto directional_light_memory = m_directionalLightBuffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	mapped = m_directionalLightBuffer->Map(directional_light_size);
	memcpy(mapped, &m_directionalLight, directional_light_size);

	m_pushConstant.ObjectColor = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);

	m_dynamicBufferObject.DynamicAlignment = sizeof(glm::mat4);
	m_dynamicBufferObject.MinAlignment = m_physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
	if (m_dynamicBufferObject.MinAlignment > 0)
	{
		m_dynamicBufferObject.DynamicAlignment = (m_dynamicBufferObject.DynamicAlignment + m_dynamicBufferObject.MinAlignment - 1) & ~(m_dynamicBufferObject.MinAlignment - 1);
	}

	auto dbo_size = m_dynamicBufferObject.DynamicAlignment * m_dynamicBufferObject.Object.Models.size();
	m_dynamicBufferObject.Object.Buffer = reinterpret_cast<glm::mat4*>(AllocateAlignedMemory(dbo_size, m_dynamicBufferObject.DynamicAlignment));
	assert(m_dynamicBufferObject.Object.Buffer);
	m_dynamicUniformBuffer = std::make_unique<Buffer>(m_logicalDevice, vk::BufferUsageFlagBits::eUniformBuffer, dbo_size);
	m_dynamicUniformBuffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

	for (auto i = 0; i < m_dynamicBufferObject.Object.Models.size(); ++i)
	{
		auto ptr = reinterpret_cast<glm::mat4*>(reinterpret_cast<uint64_t>(m_dynamicBufferObject.Object.Buffer) + (m_dynamicBufferObject.DynamicAlignment * i));

		*ptr = m_dynamicBufferObject.Object.Models[m_dynamicBufferObject.Object.ModelIndices[i]];
	}

	mapped = m_dynamicUniformBuffer->Map(VK_WHOLE_SIZE);
	memcpy(mapped, m_dynamicBufferObject.Object.Buffer, m_dynamicUniformBuffer->GetBufferSize());
}

GLVK::VK::SwapchainDetails GLVK::VK::GraphicsEngine::GetSwapchainDetails(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) {
    auto details = SwapchainDetails();
    details.SurfaceCapabilities = device.getSurfaceCapabilitiesKHR(surface);
    details.Formats = device.getSurfaceFormatsKHR(surface);
    details.PresentModes = device.getSurfacePresentModesKHR(surface);
    return details;
}

vk::Extent2D GLVK::VK::GraphicsEngine::GetExtent(const vk::SurfaceCapabilitiesKHR &capabilities, GLFWwindow* handle) noexcept {
    if (capabilities.minImageExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        auto width = 0;
        auto height = 0;
        glfwGetFramebufferSize(handle, &width, &height);

        auto extent = vk::Extent2D();
        extent.width = std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width
        );
        extent.height = std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height
        );
        return extent;
    }
}

vk::SurfaceFormatKHR GLVK::VK::GraphicsEngine::GetSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &formats) noexcept {
    for (const auto& format : formats)
    {
        if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return format;
        }
    }
    return formats[0];
}

vk::PresentModeKHR
GLVK::VK::GraphicsEngine::GetPresentMode(const std::vector<vk::PresentModeKHR> &presentModes) noexcept {
    auto fifo_support = false;
    for (const auto& mode : presentModes)
    {
        if (mode == vk::PresentModeKHR::eMailbox)
            return mode;
        else if (mode == vk::PresentModeKHR::eFifo)
            fifo_support = true;
    }
    return fifo_support ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate;
}

vk::Format GLVK::VK::GraphicsEngine::ChooseDepthFormat(const vk::PhysicalDevice& physicalDevice, const std::vector<vk::Format>& formats, const vk::ImageTiling& imageTiling, const vk::FormatFeatureFlags& formatFeatures) noexcept
{
	for (const auto& format : formats)
	{
		auto properties = physicalDevice.getFormatProperties(format);
		if (imageTiling == vk::ImageTiling::eLinear && ((properties.linearTilingFeatures & formatFeatures) == formatFeatures))
		{
			return format;
		}
		else if (imageTiling == vk::ImageTiling::eOptimal && ((properties.optimalTilingFeatures & formatFeatures) == formatFeatures))
		{
			return format;
		}
	}
	return formats[0];
}

vk::Format GLVK::VK::GraphicsEngine::GetDepthFormat(const vk::PhysicalDevice& physicalDevice, const vk::ImageTiling& imageTiling) noexcept
{
	return ChooseDepthFormat(physicalDevice, { vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint }, imageTiling, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::SampleCountFlagBits GLVK::VK::GraphicsEngine::GetMsaaSampleCounts(const vk::PhysicalDevice& physicalDevice)
{
	auto properties = physicalDevice.getProperties();
	auto sample_counts = std::min(properties.limits.sampledImageColorSampleCounts, properties.limits.sampledImageDepthSampleCounts);

	if (sample_counts & vk::SampleCountFlagBits::e64) return vk::SampleCountFlagBits::e64;
	if (sample_counts & vk::SampleCountFlagBits::e32) return vk::SampleCountFlagBits::e32;
	if (sample_counts & vk::SampleCountFlagBits::e16) return vk::SampleCountFlagBits::e16;
	if (sample_counts & vk::SampleCountFlagBits::e8) return vk::SampleCountFlagBits::e8;
	if (sample_counts & vk::SampleCountFlagBits::e4) return vk::SampleCountFlagBits::e4;
	if (sample_counts & vk::SampleCountFlagBits::e2) return vk::SampleCountFlagBits::e2;
	return vk::SampleCountFlagBits::e1;
}

void GLVK::VK::GraphicsEngine::CreateFramebuffers()
{
    m_framebuffers.resize(m_images.size());
    for (auto i = 0; i < m_images.size(); ++i)
    {
        vk::ImageView image_views[] =
        {
                m_msaaImage->GetImageView(),
                m_depthImage->GetImageView(),
                m_images[i]->GetImageView()
        };

        auto info = vk::FramebufferCreateInfo();
        info.attachmentCount = 3;
        info.pAttachments = image_views;
        info.height = m_extent.height;
        info.layers = 1;
        info.renderPass = m_pipeline->GetRenderPass();
        info.width = m_extent.width;
        m_framebuffers[i] = m_logicalDevice.createFramebuffer(info);
    }
}

void GLVK::VK::GraphicsEngine::CreateCommandBuffers()
{
    auto info = vk::CommandBufferAllocateInfo();
    info.commandBufferCount = static_cast<uint32_t>(m_images.size());
    info.commandPool = m_commandPool;
    info.level = vk::CommandBufferLevel::ePrimary;
    m_commandBuffers = m_logicalDevice.allocateCommandBuffers(info);
}

void GLVK::VK::GraphicsEngine::BeginRenderPass()
{
    static const auto clear_color = vk::ClearColorValue(std::array<float, 4>{ 1.0f, 1.0f, 1.0f, 1.0f });
    static const auto clear_depth = vk::ClearDepthStencilValue(1.0f, 0);
    static const vk::ClearValue clear_values[] = { vk::ClearValue(clear_color), vk::ClearValue(clear_depth) };
    auto renderpass_info = vk::RenderPassBeginInfo();
    renderpass_info.renderPass = m_pipeline->GetRenderPass();
    renderpass_info.renderArea.extent = m_extent;
    renderpass_info.renderArea.offset = vk::Offset2D();
    renderpass_info.clearValueCount = _countof(clear_values);
    renderpass_info.pClearValues = clear_values;
    auto begin_info = vk::CommandBufferBeginInfo();
    begin_info.pInheritanceInfo = nullptr;

    auto scissor = vk::Rect2D();
    scissor.extent = m_extent;

    for (auto i = 0; i < m_commandBuffers.size(); ++i)
    {
        renderpass_info.framebuffer = m_framebuffers[i];
        m_commandBuffers[i].begin(begin_info);
        m_commandBuffers[i].beginRenderPass(renderpass_info, vk::SubpassContents::eInline);

		m_commandBuffers[i].setViewport(0, { vk::Viewport(0.0f, 0.0f, static_cast<float>(m_extent.width), static_cast<float>(m_extent.height), 0.0f, 1.0f) });
        m_commandBuffers[i].setScissor(0, { scissor });
		/*m_commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipeline->GetPipelineLayout(), 0, { m_descriptorSet }, { 0 });*/
        m_commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->GetPipeline(BlendMode::None));

		//for (const auto& mesh : m_meshes)
		//{
		//	/*if (!mesh->Textures.empty())
		//	{
		//		m_pushConstant.TextureIndex = mesh->TextureIndices[0];
		//	}*/

		//	m_pushConstant.ObjectColor = mesh->Color;
		//	m_commandBuffers[i].pushConstants<PushConstant>(m_pipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, { m_pushConstant });
		//	m_commandBuffers[i].bindVertexBuffers(0, mesh->VertexBuffer->GetBuffer(), { 0 });
		//	m_commandBuffers[i].bindIndexBuffer(mesh->IndexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
		//	m_commandBuffers[i].drawIndexed(static_cast<uint32_t>(mesh->Indices.size()), 1, 0, 0, 0);
		//}

		//for (auto j = 0; j < m_models.size(); ++j)
		//{
		//	uint32_t dynamic_offset = m_dynamicBufferObject.Object.ModelIndices[j] * static_cast<uint32_t>(m_dynamicBufferObject.DynamicAlignment);

		//	m_commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipeline->GetPipelineLayout(), 0, { m_descriptorSet }, { dynamic_offset });

		//	for (const auto& mesh : m_models[j]->Meshes)
		//	{
		//		/*if (!mesh.Textures.empty())
		//		{
		//			m_pushConstant.TextureIndex = mesh.TextureIndices[0];
		//		}*/

		//		m_pushConstant.ObjectColor = m_models[j]->Color;
		//		m_commandBuffers[i].pushConstants<PushConstant>(m_pipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, { m_pushConstant });
		//		m_commandBuffers[i].bindVertexBuffers(0, mesh.VertexBuffer->GetBuffer(), { 0 });
		//		m_commandBuffers[i].bindIndexBuffer(mesh.IndexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
		//		m_commandBuffers[i].drawIndexed(static_cast<uint32_t>(mesh.Indices.size()), 1, 0, 0, 0);
		//	}
		//}

        m_commandBuffers[i].endRenderPass();
        m_commandBuffers[i].end();
    }
}

void* GLVK::VK::GraphicsEngine::CreateCube(const Vector3& position, const Vector3& scale, const Vector3& rotation, const Vector4& color)
{
	static const std::vector<Vertex> cube_vertices =
	{
		// Front Face
		{ { -0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },

		// Top Face
		{ { -0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },

		// Back Face
		{ {  0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },

		// Bottom Face
		{ { -0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },

		// Left Face
		{ { -0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },

		// Right Face
		{ {  0.5f,  0.5f, -0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
	};

	static const std::vector<uint32_t> cube_indices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,

		8, 9, 10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,

		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20
	};

	auto& mesh = m_meshes.emplace_back(std::make_unique<MESH>());
	mesh->Vertices = cube_vertices;
	mesh->Indices = cube_indices;
	mesh->VertexBuffer = std::dynamic_pointer_cast<Buffer>(CreateVertexBuffer(mesh->Vertices));
	mesh->IndexBuffer = std::dynamic_pointer_cast<Buffer>(CreateIndexBuffer(mesh->Indices));
	mesh->Position = position;
	mesh->ScaleX = scale.x;
	mesh->ScaleY = scale.y;
	mesh->ScaleZ = scale.z;
	mesh->RotationX = rotation.x;
	mesh->RotationY = rotation.y;
	mesh->RotationZ = rotation.z;
	mesh->Color = color;
	return mesh.get();
}

void* GLVK::VK::GraphicsEngine::CreateSphere()
{
	return nullptr;
}

void* GLVK::VK::GraphicsEngine::CreateCylinder()
{
	return nullptr;
}

void* GLVK::VK::GraphicsEngine::CreateCapsule()
{
	return nullptr;
}

void GLVK::VK::GraphicsEngine::CreateSynchronizationObjects()
{
    m_imageAcquiredSemaphores.resize(m_images.size());
    m_renderCompletedSemaphores.resize(m_images.size());
    m_fences.resize(m_images.size());

    auto semaphore_info = vk::SemaphoreCreateInfo();
    auto fence_info = vk::FenceCreateInfo();
    fence_info.flags = vk::FenceCreateFlagBits::eSignaled;

    for (auto i = 0; i < m_images.size(); ++i)
    {
        m_imageAcquiredSemaphores[i] = m_logicalDevice.createSemaphore(semaphore_info);
        m_renderCompletedSemaphores[i] = m_logicalDevice.createSemaphore(semaphore_info);
        m_fences[i] = m_logicalDevice.createFence(fence_info);
    }
}