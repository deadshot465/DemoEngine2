#pragma once
#include "../../Interfaces/IDisposable.h"
#include "../../Interfaces/IMappableVK.h"

namespace GLVK
{
	namespace VK
	{
		class Buffer
			: public IMappable, public IDisposable
		{
		public:
			Buffer(const vk::Device& device, const vk::BufferUsageFlags& bufferUsage, vk::DeviceSize size);
			virtual ~Buffer();

			virtual void Dispose() override;
			void CopyBufferToBuffer(const vk::Buffer& srcBuffer, vk::DeviceSize size, const vk::CommandPool& commandPool, const vk::Queue& graphicsQueue);
			void CopyBufferToImage(const vk::Image& targetImage, uint32_t height, uint32_t width, vk::DeviceSize size, const vk::ImageAspectFlags& imageFlags, vk::CommandPool& commandPool, const vk::Queue& graphicsQueue);
			virtual const vk::DeviceMemory& AllocateMemory(const vk::PhysicalDevice& physicalDevice, const vk::MemoryPropertyFlags& memoryProperties) override;
			const vk::Buffer& GetBuffer() const noexcept
            {
			    return m_buffer;
            }

		private:
			vk::Buffer m_buffer = nullptr;
		};
	}
}

