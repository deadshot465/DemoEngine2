#include "ImageVK.h"
#include <utility>
#include <cassert>
#include "UtilsVK.h"

GLVK::VK::Image::Image(const vk::Device& device)
	: IMappable(device)
{
}

GLVK::VK::Image::Image(const vk::Device& device, vk::Image& image)
	: IMappable(device), m_image(std::move(image))
{
}

GLVK::VK::Image::Image(const vk::Device& device, const vk::Format& format, const vk::SampleCountFlagBits& sampleCount, const vk::Extent2D& extent, const vk::ImageType& imageType, uint32_t mipLevels, const vk::ImageUsageFlags& imageUsage)
	: IMappable(device)
{
	auto info = vk::ImageCreateInfo();
	info.arrayLayers = 1;
	info.extent.depth = 1;
	info.extent.height = extent.height;
	info.extent.width = extent.width;
	info.format = format;
	info.imageType = imageType;
	info.initialLayout = vk::ImageLayout::eUndefined;
	info.mipLevels = mipLevels;
	info.pQueueFamilyIndices = nullptr;
	info.queueFamilyIndexCount = 0;
	info.samples = sampleCount;
	info.sharingMode = vk::SharingMode::eExclusive;
	info.tiling = vk::ImageTiling::eOptimal;
	info.usage = imageUsage;

	m_image = m_logicalDevice.createImage(info);
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

void GLVK::VK::Image::TransitionLayout(const vk::ImageLayout& srcLayout, const vk::ImageLayout& dstLayout, const vk::CommandPool& commandPool, const vk::Queue& graphicsQueue, const vk::ImageAspectFlags& imageAspects, uint32_t levelCount)
{
	auto cmd_buffer = CreateSingleTimeBuffer(m_logicalDevice, commandPool);
	auto old_stage = vk::PipelineStageFlagBits();
	auto new_stage = vk::PipelineStageFlagBits();

	auto barrier = vk::ImageMemoryBarrier();
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_image;
	barrier.newLayout = dstLayout;
	barrier.oldLayout = srcLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = imageAspects;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = levelCount;

	if (srcLayout == vk::ImageLayout::eUndefined && dstLayout == vk::ImageLayout::eDepthAttachmentOptimal)
	{
		new_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		old_stage = vk::PipelineStageFlagBits::eTopOfPipe;
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	}
	else if (srcLayout == vk::ImageLayout::eUndefined && dstLayout == vk::ImageLayout::eColorAttachmentOptimal)
	{
		new_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		old_stage = vk::PipelineStageFlagBits::eTopOfPipe;
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
	}

	cmd_buffer.pipelineBarrier(old_stage, new_stage, {}, {}, {}, barrier);
	ExecuteCommandBuffer(cmd_buffer, m_logicalDevice, commandPool, graphicsQueue);
}

const vk::DeviceMemory& GLVK::VK::Image::AllocateMemory(const vk::PhysicalDevice& physicalDevice, const vk::MemoryPropertyFlags& memoryProperties)
{
	auto requirements = m_logicalDevice.getImageMemoryRequirements(m_image);
	MapDeviceMemory(requirements, physicalDevice, memoryProperties);
	m_logicalDevice.bindImageMemory(m_image, m_deviceMemory, 0);
	return m_deviceMemory;
}
