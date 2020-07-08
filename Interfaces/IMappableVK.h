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

		protected:
			vk::Device m_logicalDevice = nullptr;
			vk::DeviceMemory m_deviceMemory = nullptr;
		};
	}
}

