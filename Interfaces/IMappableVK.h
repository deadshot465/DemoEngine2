#pragma once
#include <vulkan/vulkan.hpp>

namespace GLVK
{
	namespace VK
	{
		class IMappable
		{
		public:
			IMappable(const vk::Device& device)
				: m_logicalDevice(device)
			{

			}
			
			virtual ~IMappable()
			{
				if (m_deviceMemory)
				{
					m_logicalDevice.unmapMemory(m_deviceMemory);
				}
			}

			uint32_t GetMemoryTypeIndex(const vk::PhysicalDevice& physicalDevice, uint32_t memoryType, const vk::MemoryPropertyFlags& memoryProperties)
			{
				auto properties = physicalDevice.getMemoryProperties();
				for (uint32_t i = 0; i < properties.memoryTypeCount; ++i)
				{
					if ((memoryType & (1 << i)) &&
						(properties.memoryTypes[i].propertyFlags & memoryProperties) == memoryProperties)
					{
						return i;
					}
				}
				return 0;
			}

			const vk::DeviceMemory& MapDeviceMemory(const vk::MemoryRequirements& requirements)
			{
				auto allocate_info = vk::MemoryAllocateInfo();
				allocate_info.allocationSize = requirements.size;
				requirements.
			}

		protected:
			vk::Device m_logicalDevice = nullptr;
			vk::DeviceMemory m_deviceMemory = nullptr;
		};
	}
}

