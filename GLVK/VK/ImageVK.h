#pragma once
#include <vulkan/vulkan.hpp>
#include "../../Interfaces/IMappableVK.h"

namespace GLVK
{
	namespace VK
	{
		class Image
			: public IMappable
		{
		public:
			Image(const vk::Device& device);
			Image(const vk::Device& device, vk::Image& image);
			Image(const vk::Device& device, const vk::Format& format, const vk::SampleCountFlagBits& sampleCount, const vk::Extent2D& extent, const vk::ImageType& imageType, uint32_t mipLevels, const vk::ImageUsageFlags& imageUsage);
			virtual ~Image();

			virtual const vk::DeviceMemory& AllocateMemory(const vk::PhysicalDevice& physicalDevice, const vk::MemoryPropertyFlags& memoryProperties) override;

			void CreateImageView(const vk::Format& format, const vk::ImageAspectFlags& aspectMask, uint32_t levelCount, const vk::ImageViewType& imageViewType);
			void TransitionLayout(const vk::ImageLayout& srcLayout, const vk::ImageLayout& dstLayout, const vk::CommandPool& commandPool, const vk::Queue& graphicsQueue, const vk::ImageAspectFlags& imageAspects, uint32_t levelCount);

		private:
			vk::Image m_image = nullptr;
			vk::ImageView m_imageView = nullptr;
		};
	}
}

