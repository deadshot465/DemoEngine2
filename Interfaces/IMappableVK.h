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
				if (m_mappedMemory)
					UnMap();

				if (m_deviceMemory)
				{
					m_logicalDevice.freeMemory(m_deviceMemory);
				}
			}

			virtual const vk::DeviceMemory& AllocateMemory(const vk::PhysicalDevice& physicalDevice, const vk::MemoryPropertyFlags& memoryProperties) = 0;

			void* Map(vk::DeviceSize size, vk::DeviceSize offset = 0)
			{
				if (m_mappedMemory) return m_mappedMemory;
				m_mappedMemory = m_logicalDevice.mapMemory(m_deviceMemory, offset, size);
				return m_mappedMemory;
			}

			void UnMap()
			{
				m_logicalDevice.unmapMemory(m_deviceMemory);
				m_mappedMemory = nullptr;
			}

			[[nodiscard]] const vk::DeviceMemory& GetDeviceMemory() const noexcept
            {
			    return m_deviceMemory;
            }

			[[nodiscard]] const void* GetMappedMemory() const noexcept
			{
				return m_mappedMemory;
			}

		protected:
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

			const vk::DeviceMemory& MapDeviceMemory(const vk::MemoryRequirements& requirements, const vk::PhysicalDevice& physicalDevice, const vk::MemoryPropertyFlags& memoryProperties)
			{
				auto allocate_info = vk::MemoryAllocateInfo();
				allocate_info.allocationSize = requirements.size;
				allocate_info.memoryTypeIndex = GetMemoryTypeIndex(physicalDevice, requirements.memoryTypeBits, memoryProperties);
				m_deviceMemory = m_logicalDevice.allocateMemory(allocate_info);
				return m_deviceMemory;
			}

		protected:
			vk::Device m_logicalDevice = nullptr;
			vk::DeviceMemory m_deviceMemory = nullptr;
			void* m_mappedMemory = nullptr;
		};
	}
}

