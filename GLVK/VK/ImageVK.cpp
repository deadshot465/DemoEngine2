#include "ImageVK.h"
#include <utility>
#include <cassert>

GLVK::VK::Image::Image(const vk::Device& device)
	: IMappable(device)
{
}

GLVK::VK::Image::Image(const vk::Device& device, vk::Image& image)
	: IMappable(device), m_image(std::move(image))
{
}

GLVK::VK::Image::~Image()
{
	if (m_imageView)
	{
		m_logicalDevice.destroyImageView(m_imageView);
	}

	if (m_deviceMemory)
	{
		m_logicalDevice.destroyImage(m_image);
	}
}

void GLVK::VK::Image::CreateImageView(const vk::Format& format, const vk::ImageAspectFlags& aspectMask, uint32_t levelCount, const vk::ImageViewType& imageViewType)
{
	auto info = vk::ImageViewCreateInfo();
	info.components.r = vk::ComponentSwizzle::eIdentity;
	info.components.g = vk::ComponentSwizzle::eIdentity;
	info.components.b = vk::ComponentSwizzle::eIdentity;
	info.components.a = vk::ComponentSwizzle::eIdentity;

	info.format = format;
	info.image = m_image;
	info.subresourceRange.aspectMask = aspectMask;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.layerCount = 1;
	info.subresourceRange.levelCount = levelCount;
	info.viewType = imageViewType;

	m_imageView = m_logicalDevice.createImageView(info);
}
