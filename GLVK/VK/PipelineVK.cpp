#include "PipelineVK.h"

GLVK::VK::Pipeline::Pipeline(const vk::Device& device)
	: m_logicalDevice(device)
{
}

GLVK::VK::Pipeline::~Pipeline()
{
	if (m_ownedRenderPass)
		m_logicalDevice.destroyRenderPass(m_renderPass);
}

void GLVK::VK::Pipeline::CreateRenderPass(const vk::Format& graphicsFormat, const vk::Format& depthFormat, const vk::SampleCountFlagBits& sampleCount)
{
	vk::AttachmentDescription attachments[3] = {};
	attachments[0].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
	attachments[0].format = graphicsFormat;
	attachments[0].initialLayout = vk::ImageLayout::eUndefined;
	attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[0].samples = sampleCount;
	attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[0].storeOp = vk::AttachmentStoreOp::eStore;

	attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	attachments[1].format = depthFormat;
	attachments[1].initialLayout = vk::ImageLayout::eUndefined;
	attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[1].samples = sampleCount;
	attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[1].storeOp = vk::AttachmentStoreOp::eStore;

	attachments[2].finalLayout = vk::ImageLayout::ePresentSrcKHR;
	attachments[2].format = graphicsFormat;
	attachments[2].initialLayout = vk::ImageLayout::eUndefined;
	attachments[2].loadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[2].samples = vk::SampleCountFlagBits::e1;
	attachments[2].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[2].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[2].storeOp = vk::AttachmentStoreOp::eStore;

	auto subpass_dependency = vk::SubpassDependency();
	subpass_dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
	subpass_dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	subpass_dependency.dstSubpass = 0;
	subpass_dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
	subpass_dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	
	auto color_attachment = vk::AttachmentReference();
	color_attachment.attachment = 0;
	color_attachment.layout = vk::ImageLayout::eColorAttachmentOptimal;
	
	auto depth_attachment = vk::AttachmentReference();
	depth_attachment.attachment = 1;
	depth_attachment.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	auto resolve_attachment = vk::AttachmentReference();
	resolve_attachment.attachment = 2;
	resolve_attachment.layout = vk::ImageLayout::eColorAttachmentOptimal;

	auto subpass_description = vk::SubpassDescription();
	subpass_description.colorAttachmentCount = 1;
	subpass_description.inputAttachmentCount = 0;
	subpass_description.pColorAttachments = &color_attachment;
	subpass_description.pDepthStencilAttachment = &depth_attachment;
	subpass_description.pInputAttachments = nullptr;
	subpass_description.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass_description.pPreserveAttachments = nullptr;
	subpass_description.pResolveAttachments = &resolve_attachment;

	auto info = vk::RenderPassCreateInfo();
	info.attachmentCount = _countof(attachments);
	info.dependencyCount = 1;
	info.pAttachments = attachments;
	info.pDependencies = &subpass_dependency;
	info.pSubpasses = &subpass_description;
	info.subpassCount = 1;

	m_renderPass = m_logicalDevice.createRenderPass(info);
	m_ownedRenderPass = true;
}

void GLVK::VK::Pipeline::CreateGraphicPipeline()
{
}

void GLVK::VK::Pipeline::CreateComputePipeline()
{
}
