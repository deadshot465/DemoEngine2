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
			virtual ~Image();

			void CreateImageView(const vk::Format& format, const vk::ImageAspectFlags& aspectMask, uint32_t levelCount, const vk::ImageViewType& imageViewType);

		private:
			vk::Image m_image = nullptr;
			vk::ImageView m_imageView = nullptr;
		};
	}
}

