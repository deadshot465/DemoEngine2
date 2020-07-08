#pragma once
#include "../../Interfaces/IMappableVK.h"

namespace GLVK
{
	namespace VK
	{
		class Buffer
			: public IMappable
		{
		public:
			Buffer(const vk::Device& device, const vk::BufferUsageFlags& bufferUsage, vk::DeviceSize size);
			virtual ~Buffer();

			void CopyBufferToBuffer(const vk::Buffer& srcBuffer, vk::DeviceSize& size, const vk::CommandPool& commandPool, const vk::Queue& graphicsQueue);

		private:
			vk::Buffer m_buffer = nullptr;
		};
	}
}
