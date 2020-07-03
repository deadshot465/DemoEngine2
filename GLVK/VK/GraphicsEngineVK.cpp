#include "GraphicsEngineVK.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include "UtilsVK.h"

GLVK::VK::GraphicsEngine::GraphicsEngine(GLFWwindow* window, int width, int height)
	: m_handle(window), m_width(width), m_height(height)
{
	try
	{
		CreateInstance();
		SetupDebug();
		CreateSurface();
		GetPhysicalDevice();
	}
	catch (const std::exception&)
	{
		throw;
	}
}

GLVK::VK::GraphicsEngine::~GraphicsEngine()
{
	Dispose();
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

std::vector<const char*> GLVK::VK::GraphicsEngine::GetRequiredExtensions(bool debug)
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

bool GLVK::VK::GraphicsEngine::CheckLayerSupport()
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

bool GLVK::VK::GraphicsEngine::IsDeviceSuitable(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface)
{
	auto queue_indices = GetQueueIndices(device, surface);
	if (!queue_indices.IsCompleted())
		return false;
	
	auto properties = device.getProperties();
	std::cout << "Device: " << properties.deviceName << '\n';

	auto features = device.getFeatures();
	auto feature_supports = features.samplerAnisotropy && features.sampleRateShading;

	return feature_supports;
}

GLVK::VK::QueueIndices GLVK::VK::GraphicsEngine::GetQueueIndices(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface)
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

void GLVK::VK::GraphicsEngine::Dispose()
{

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
