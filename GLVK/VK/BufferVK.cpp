#include "BufferVK.h"
#include "UtilsVK.h"

GLVK::VK::Buffer::Buffer(const vk::Device& device, const vk::BufferUsageFlags& bufferUsage, vk::DeviceSize size)
	: IMappable(device)
{
	auto info = vk::BufferCreateInfo();
	info.pQueueFamilyIndices = nullptr;
	info.queueFamilyIndexCount = 0;
	info.sharingMode = vk::SharingMode::eExclusive;
	info.size = size;
	info.usage = bufferUsage;
	
	m_buffer = device.createBuffer(info);
}

GLVK::VK::Buffer::~Buffer()
{
	if (!m_isDisposed) Dispose();
}

void GLVK::VK::Buffer::CopyBufferToBuffer(const vk::Buffer& srcBuffer, vk::DeviceSize size, const vk::CommandPool& commandPool, const vk::Queue& graphicsQueue)
{
	auto info = vk::BufferCopy();
	info.dstOffset = 0;
	info.size = size;
	info.srcOffset = 0;

	auto cmd_buffer = CreateSingleTimeBuffer(m_logicalDevice, commandPool);
	cmd_buffer.copyBuffer(srcBuffer, m_buffer, info);
	ExecuteCommandBuffer(cmd_buffer, m_logicalDevice, commandPool, graphicsQueue);
}

void GLVK::VK::Buffer::CopyBufferToImage(const vk::Image& targetImage, uint32_t height, uint32_t width, vk::DeviceSize size, const vk::ImageAspectFlags& imageFlags, vk::CommandPool& commandPool, const vk::Queue& graphicsQueue)
{
	auto info = vk::BufferImageCopy();
	info.bufferImageHeight = 0;
	info.bufferOffset = 0;
	info.bufferRowLength = 0;
	info.imageExtent.depth = 1;
	info.imageExtent.height = height;
	info.imageExtent.width = width;
	info.imageOffset = {};
	info.imageSubresource.aspectMask = imageFlags;
	info.imageSubresource.baseArrayLayer = 0;
	info.imageSubresource.layerCount = 1;
	info.imageSubresource.mipLevel = 0;
	
	auto cmd_buffer = CreateSingleTimeBuffer(m_logicalDevice, commandPool);
	cmd_buffer.copyBufferToImage(m_buffer, targetImage, vk::ImageLayout::eTransferDstOptimal, info);
	ExecuteCommandBuffer(cmd_buffer, m_logicalDevice, commandPool, graphicsQueue);
}

const vk::DeviceMemory &GLVK::VK::Buffer::AllocateMemory(const vk::PhysicalDevice& physicalDevice, const vk::MemoryPropertyFlags& memoryProperties) {
    auto requirements = m_logicalDevice.getBufferMemoryRequirements(m_buffer);
	MapDeviceMemory(requirements, physicalDevice, memoryProperties);
	m_logicalDevice.bindBufferMemory(m_buffer, m_deviceMemory, 0);
	return m_deviceMemory;
}

void GLVK::VK::Buffer::Dispose()
{
	m_logicalDevice.destroyBuffer(m_buffer);
	m_isDisposed = true;
}
