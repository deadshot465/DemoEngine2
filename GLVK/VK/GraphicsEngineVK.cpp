#include "GraphicsEngineVK.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <unordered_set>
#include <numeric>
#include <cassert>
#include <cstring>
#include "BufferVK.h"
#include "ImageVK.h"
#include "PipelineVK.h"
#include "ShaderVK.h"

GLVK::VK::GraphicsEngine::GraphicsEngine(GLFWwindow* window, int width, int height)
	: m_handle(window), m_width(width), m_height(height)
{
	try
	{
		CreateInstance();
		SetupDebug();
		CreateSurface();
		GetPhysicalDevice();
		CreateLogicalDevice();
		LoadShader();
		LoadDefaultCube();

		auto pool_info = vk::CommandPoolCreateInfo();
		pool_info.queueFamilyIndex = m_queueIndices.GraphicsQueue.value();
		m_commandPool = m_logicalDevice.createCommandPool(pool_info);

		CreateBuffers();
		Initialize();
	}
	catch (const std::exception&)
	{
		throw;
	}
}

GLVK::VK::GraphicsEngine::~GraphicsEngine()
{
	Dispose();
	m_logicalDevice.destroyDescriptorSetLayout(m_descriptorSetLayout);
	m_logicalDevice.destroyCommandPool(m_commandPool);
	if (m_intermediateBuffer) m_intermediateBuffer.reset();
	m_indexBuffer.reset();
	m_vertexBuffer.reset();
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

}

void GLVK::VK::GraphicsEngine::Render()
{

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
	auto feature_supports = features.samplerAnisotropy && features.sampleRateShading;

	auto swapchain_details = GetSwapchainDetails(device, surface);
	auto is_swapchain_adequate = swapchain_details.Formats.size() > 0 && swapchain_details.PresentModes.size() > 0;

	return feature_supports && CheckExtensionSupport(device) && is_swapchain_adequate;
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
		CreateDescriptorLayout();
		CreateDescriptorSets();
		CreateDepthImage();
		CreateMultisamplingImage();
		CreateUniformBuffers();
		m_pipeline = std::make_unique<Pipeline>(m_logicalDevice);
		m_pipeline->CreateRenderPass(m_format, GetDepthFormat(m_physicalDevice, vk::ImageTiling::eOptimal), m_msaaSampleCount);
		m_pipeline->CreateGraphicPipelines(m_descriptorSetLayout, m_msaaSampleCount, {
			m_vertexShader->GetShaderStageInfo(),
			m_fragmentShader->GetShaderStageInfo()
			});
	}
	catch (const std::exception&)
	{
		throw;
	}
}

void GLVK::VK::GraphicsEngine::Dispose()
{
	m_pipeline.reset();

	for (auto i = 0; i < m_images.size(); ++i)
	{
		m_mvpBuffers[i].reset();
		m_directionalLightBuffers[i].reset();
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
	auto res = glfwCreateWindowSurface(m_instance, m_handle, nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_surface));
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

	auto info = vk::DeviceCreateInfo();
	info.enabledExtensionCount = static_cast<uint32_t>(m_enabledExtensions.size());
	info.pEnabledFeatures = &features;
	info.ppEnabledExtensionNames = m_enabledExtensions.data();
	info.pQueueCreateInfos = queue_create_infos.data();
	info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());

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
	auto details = GetSwapchainDetails(m_physicalDevice, m_surface);
	auto format = GetSurfaceFormat(details.Formats);
	auto present_mode = GetPresentMode(details.PresentModes);
	auto extent = GetExtent(details.SurfaceCapabilities, m_handle);

	uint32_t min_image_count = 0;
	if (details.SurfaceCapabilities.maxImageCount > 0)
	{
		min_image_count = std::clamp<uint32_t>(details.SurfaceCapabilities.minImageCount + 1, details.SurfaceCapabilities.minImageCount, details.SurfaceCapabilities.maxImageCount);
	}
	else
	{
		min_image_count = details.SurfaceCapabilities.minImageCount + 1;
	}

	auto info = vk::SwapchainCreateInfoKHR();
	info.oldSwapchain = nullptr;
	info.clipped = VK_FALSE;
	info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	info.imageArrayLayers = 1;
	info.imageColorSpace = format.colorSpace;
	info.imageExtent = extent;
	info.imageFormat = format.format;
	info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	info.minImageCount = min_image_count;
	info.preTransform = details.SurfaceCapabilities.currentTransform;
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
	m_format = format.format;
	m_extent = extent;

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

void GLVK::VK::GraphicsEngine::LoadDefaultCube()
{
	m_cubeVertices =
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

	m_cubeIndices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,

		8, 9, 10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,

		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20
	};
}

void GLVK::VK::GraphicsEngine::CreateBuffers()
{
	CreateVertexBuffers();
	CreateIndexBuffers();
}

void GLVK::VK::GraphicsEngine::CreateVertexBuffers()
{
	static const vk::DeviceSize buffer_size = sizeof(Vertex) * m_cubeVertices.size();
	m_intermediateBuffer.reset(new Buffer(m_logicalDevice, vk::BufferUsageFlagBits::eTransferSrc, buffer_size));
	auto memory = m_intermediateBuffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	void* mapped = m_logicalDevice.mapMemory(memory, 0, buffer_size);
	memcpy(mapped, m_cubeVertices.data(), buffer_size);
	m_logicalDevice.unmapMemory(memory);

	m_vertexBuffer = std::make_unique<Buffer>(m_logicalDevice, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, buffer_size);
	m_vertexBuffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
	m_vertexBuffer->CopyBufferToBuffer(m_intermediateBuffer->GetBuffer(), buffer_size, m_commandPool, m_graphicsQueue);
}

void GLVK::VK::GraphicsEngine::CreateIndexBuffers()
{
	vk::DeviceSize buffer_size = sizeof(uint32_t) * m_cubeIndices.size();
	m_intermediateBuffer.reset(new Buffer(m_logicalDevice, vk::BufferUsageFlagBits::eTransferSrc, buffer_size));
	auto memory = m_intermediateBuffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	void* mapped = m_logicalDevice.mapMemory(memory, 0, buffer_size);
	memcpy(mapped, m_cubeIndices.data(), buffer_size);
	m_logicalDevice.unmapMemory(memory);

	m_indexBuffer = std::make_unique<Buffer>(m_logicalDevice, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, buffer_size);
	m_indexBuffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
	m_indexBuffer->CopyBufferToBuffer(m_intermediateBuffer->GetBuffer(), buffer_size, m_commandPool, m_graphicsQueue);
}

void GLVK::VK::GraphicsEngine::CreateDescriptorLayout()
{
	vk::DescriptorSetLayoutBinding bindings[2] = {};
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

	auto info = vk::DescriptorSetLayoutCreateInfo();
	info.bindingCount = static_cast<uint32_t>(_countof(bindings));
	info.pBindings = bindings;
	m_descriptorSetLayout = m_logicalDevice.createDescriptorSetLayout(info);
}

void GLVK::VK::GraphicsEngine::CreateDescriptorSets()
{
	vk::DescriptorPoolSize pool_sizes[2] = {};
	pool_sizes[0].descriptorCount = 1;
	pool_sizes[0].type = vk::DescriptorType::eUniformBuffer;
	pool_sizes[1].descriptorCount = 1;
	pool_sizes[1].type = vk::DescriptorType::eUniformBuffer;

	assert(m_images.size() > 0);
	auto pool_info = vk::DescriptorPoolCreateInfo();
	pool_info.maxSets = static_cast<uint32_t>(m_images.size());
	pool_info.poolSizeCount = _countof(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	m_descriptorPool = m_logicalDevice.createDescriptorPool(pool_info);

	auto set_layouts = std::vector<vk::DescriptorSetLayout>(m_images.size(), m_descriptorSetLayout);
	auto allocate_info = vk::DescriptorSetAllocateInfo();
	allocate_info.descriptorPool = m_descriptorPool;
	allocate_info.descriptorSetCount = static_cast<uint32_t>(set_layouts.size());
	allocate_info.pSetLayouts = set_layouts.data();
	m_descriptorSets = m_logicalDevice.allocateDescriptorSets(allocate_info);

	for (auto i = 0; i < m_descriptorSets.size(); ++i)
	{
		auto mvp_buffer_info = vk::DescriptorBufferInfo();
		mvp_buffer_info.buffer = m_mvpBuffers[i]->GetBuffer();
		mvp_buffer_info.offset = 0;
		mvp_buffer_info.range = sizeof(MVP);
		
		auto directional_light_buffer_info = vk::DescriptorBufferInfo();
		directional_light_buffer_info.buffer = m_directionalLightBuffers[i]->GetBuffer();
		directional_light_buffer_info.offset = 0;
		directional_light_buffer_info.range = sizeof(DirectionalLight);

		auto write_descriptors = std::vector<vk::WriteDescriptorSet>(2);
		write_descriptors[0].descriptorCount = 1;
		write_descriptors[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		write_descriptors[0].dstArrayElement = 0;
		write_descriptors[0].dstBinding = 0;
		write_descriptors[0].dstSet = m_descriptorSets[i];
		write_descriptors[0].pBufferInfo = &mvp_buffer_info;
		write_descriptors[0].pImageInfo = nullptr;
		write_descriptors[0].pTexelBufferView = nullptr;
		
		write_descriptors[1].descriptorCount = 1;
		write_descriptors[1].descriptorType = vk::DescriptorType::eUniformBuffer;
		write_descriptors[1].dstArrayElement = 0;
		write_descriptors[1].dstBinding = 1;
		write_descriptors[1].dstSet = m_descriptorSets[i];
		write_descriptors[1].pBufferInfo = &directional_light_buffer_info;
		write_descriptors[1].pImageInfo = nullptr;
		write_descriptors[1].pTexelBufferView = nullptr;

		m_logicalDevice.updateDescriptorSets(write_descriptors, {});
	}
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

	auto rotate_x = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	auto rotate_y = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	auto rotate_z = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_mvp.Model = rotate_z * rotate_y * rotate_x * glm::mat4(1.0f);
	m_mvp.View = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	m_mvp.Projection = glm::perspective(glm::radians(45.0f), static_cast<float>(m_width) / static_cast<float>(m_height), 0.1f, 100.0f);

	m_mvpBuffers.resize(m_images.size());
	for (auto& buffer : m_mvpBuffers)
	{
		buffer = std::make_unique<Buffer>(m_logicalDevice, vk::BufferUsageFlagBits::eUniformBuffer, mvp_size);
		auto mvp_memory = buffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		auto mapped = m_logicalDevice.mapMemory(mvp_memory, 0, mvp_size);
		memcpy(mapped, &m_mvp, mvp_size);
		m_logicalDevice.unmapMemory(mvp_memory);
	}
	
	m_directionalLight.AmbientIntensity = 0.1f;
	m_directionalLight.SpecularIntensity = 0.5f;
	m_directionalLight.Diffuse = glm::vec4(1.0f);
	m_directionalLight.LightDirection = glm::vec3(0.0f, -5.0f, 0.0f);
	
	m_directionalLightBuffers.resize(m_images.size());
	for (auto& buffer : m_directionalLightBuffers)
	{
		buffer = std::make_unique<Buffer>(m_logicalDevice, vk::BufferUsageFlagBits::eUniformBuffer, directional_light_size);
		auto directional_light_memory = buffer->AllocateMemory(m_physicalDevice, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		auto mapped = m_logicalDevice.mapMemory(directional_light_memory, 0, directional_light_size);
		memcpy(mapped, &m_directionalLight, directional_light_size);
		m_logicalDevice.unmapMemory(directional_light_memory);
	}
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


