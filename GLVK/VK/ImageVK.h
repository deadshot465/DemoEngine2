#pragma once
#include <vulkan/vulkan.hpp>
#include "../../Interfaces/IDisposable.h"
#include "../../Interfaces/IMappableVK.h"

namespace GLVK
{
	namespace VK
	{
		class Image
			: public IDisposable, public IMappable
		{
		public:
			Image(const vk::Device& device);
			Image(const vk::Device& device, vk::Image& image);
			Image(const vk::Device& device, const vk::Format& format, const vk::SampleCountFlagBits& sampleCount, const vk::Extent2D& extent, const vk::ImageType& imageType, uint32_t mipLevels, const vk::ImageUsageFlags& imageUsage);
			virtual ~Image();

			virtual void Dispose() override;

			virtual const vk::DeviceMemory& AllocateMemory(const vk::PhysicalDevice& physicalDevice, const vk::MemoryPropertyFlags& memoryProperties) override;

			void CreateImageView(const vk::Format& format, const vk::ImageAspectFlags& aspectMask, uint32_t levelCount, const vk::ImageViewType& imageViewType);
			void TransitionLayout(const vk::ImageLayout& srcLayout, const vk::ImageLayout& dstLayout, const vk::CommandPool& commandPool, const vk::Queue& graphicsQueue, const vk::ImageAspectFlags& imageAspects, uint32_t levelCount);
			void GenerateMipmaps(const vk::CommandPool& commandPool, const vk::Queue& graphicsQueue, uint32_t levelCount);
			void CreateSampler(uint32_t levelCount);

			[[nodiscard]] const vk::Image& GetImage() const noexcept
			{
				return m_image;
			}

			[[nodiscard]] const vk::ImageView& GetImageView() const noexcept
            {
			    return m_imageView;
            }

			[[nodiscard]] const vk::Sampler GetSampler() const noexcept
			{
				return m_sampler;
			}

		private:
			vk::Image m_image = nullptr;
			vk::ImageView m_imageView = nullptr;
			vk::Sampler m_sampler = nullptr;
			uint32_t m_width = 0;
			uint32_t m_height = 0;
		};
	}
}

