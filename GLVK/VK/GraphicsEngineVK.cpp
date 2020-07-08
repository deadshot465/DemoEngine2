#include "GraphicsEngineVK.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_set>
#include <numeric>
#include <cassert>
#include <cstring>
#include "ImageVK.h"
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
		CreateSwapchain();
		LoadShader();
	}
	catch (const std::exception&)
	{
		throw;
	}
}

GLVK::VK::GraphicsEngine::~GraphicsEngine()
{
	Dispose();
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

void GLVK::VK::GraphicsEngine::Dispose()
{
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
	auto format = GetFormat(details.Formats);
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

vk::SurfaceFormatKHR GLVK::VK::GraphicsEngine::GetFormat(const std::vector<vk::SurfaceFormatKHR> &formats) noexcept {
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
