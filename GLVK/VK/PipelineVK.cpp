#include "PipelineVK.h"
#include <future>
#include <utility>
#include <vector>
#include "UtilsVK.h"

GLVK::VK::Pipeline::Pipeline(const vk::Device& device)
	: m_logicalDevice(device)
{
	
}

GLVK::VK::Pipeline::~Pipeline()
{
	for (auto& pipeline : m_graphicsPipelines)
	{
		m_logicalDevice.destroyPipeline(pipeline);
	}

	m_logicalDevice.destroyPipelineLayout(m_pipelineLayout);

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

void GLVK::VK::Pipeline::CreateGraphicPipeline(const vk::Device& device, const vk::PipelineColorBlendAttachmentState& colorBlendAttachment, const vk::SampleCountFlagBits& sampleCounts, const std::vector<vk::PipelineShaderStageCreateInfo>& shaderStageInfos, const vk::PipelineLayout& pipelineLayout, const vk::PipelineCache& pipelineCache, size_t blendModeIndex, const vk::RenderPass& renderPass, vk::Pipeline* pPipeline)
{
	auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo();
	auto attr_desc = GetVertexInputAttributeDescription(0);
	auto binding_desc = GetVertexInputBindingDescription(0, vk::VertexInputRate::eVertex);
	vertex_input_info.pVertexAttributeDescriptions = attr_desc.data();
	vertex_input_info.pVertexBindingDescriptions = &binding_desc;
	vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attr_desc.size());
	vertex_input_info.vertexBindingDescriptionCount = 1;

	auto ia_info = vk::PipelineInputAssemblyStateCreateInfo();
	ia_info.primitiveRestartEnable = VK_FALSE;
	ia_info.topology = vk::PrimitiveTopology::eTriangleList;

	auto vp_info = vk::PipelineViewportStateCreateInfo();
	vp_info.pScissors = nullptr;
	vp_info.pViewports = nullptr;
	vp_info.scissorCount = 1;
	vp_info.viewportCount = 1;

	auto rs_info = vk::PipelineRasterizationStateCreateInfo();
	rs_info.cullMode = vk::CullModeFlagBits::eBack;
	rs_info.depthBiasEnable = VK_FALSE;
	rs_info.depthClampEnable = VK_FALSE;
	rs_info.frontFace = vk::FrontFace::eClockwise;
	rs_info.lineWidth = 1.0f;
	rs_info.polygonMode = vk::PolygonMode::eFill;
	rs_info.rasterizerDiscardEnable = VK_FALSE;

	auto color_blend_info = vk::PipelineColorBlendStateCreateInfo();
	color_blend_info.attachmentCount = 1;
	color_blend_info.logicOp = vk::LogicOp::eCopy;
	color_blend_info.logicOpEnable = VK_FALSE;
	color_blend_info.pAttachments = &colorBlendAttachment;

	auto depth_info = vk::PipelineDepthStencilStateCreateInfo();
	depth_info.depthBoundsTestEnable = VK_FALSE;
	depth_info.depthCompareOp = vk::CompareOp::eLess;
	depth_info.depthTestEnable = VK_TRUE;
	depth_info.depthWriteEnable = VK_TRUE;
	depth_info.stencilTestEnable = VK_FALSE;

	auto dynamic_states = std::vector<vk::DynamicState>{
		vk::DynamicState::eScissor,
		vk::DynamicState::eViewport
	};

	auto dynamic_info = vk::PipelineDynamicStateCreateInfo();
	dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
	dynamic_info.pDynamicStates = dynamic_states.data();

	auto msaa_info = vk::PipelineMultisampleStateCreateInfo();
	msaa_info.alphaToCoverageEnable = VK_FALSE;
	msaa_info.alphaToOneEnable = VK_FALSE;
	msaa_info.minSampleShading = .25f;
	msaa_info.pSampleMask = nullptr;
	msaa_info.rasterizationSamples = sampleCounts;
	msaa_info.sampleShadingEnable = VK_TRUE;

	auto pipeline_info = vk::GraphicsPipelineCreateInfo();
	pipeline_info.basePipelineHandle = nullptr;
	pipeline_info.basePipelineIndex = -1;
	pipeline_info.subpass = 0;
	pipeline_info.pColorBlendState = &color_blend_info;
	pipeline_info.pDepthStencilState = &depth_info;
	pipeline_info.pDynamicState = &dynamic_info;
	pipeline_info.pInputAssemblyState = &ia_info;
	pipeline_info.pMultisampleState = &msaa_info;
	pipeline_info.pRasterizationState = &rs_info;
	pipeline_info.pStages = shaderStageInfos.data();
	pipeline_info.pTessellationState = nullptr;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pViewportState = &vp_info;
	pipeline_info.renderPass = renderPass;
	pipeline_info.stageCount = static_cast<uint32_t>(shaderStageInfos.size());
	pipeline_info.layout = pipelineLayout;

	auto pipeline = device.createGraphicsPipeline(pipelineCache, pipeline_info);
	ThrowIfFailed(pipeline.result, "Failed to create graphics pipeline.\n");

	{
		auto lock = std::lock_guard<std::mutex>{ m_mutex };
		*pPipeline = pipeline.value;
	}
}

void GLVK::VK::Pipeline::CreateGraphicPipelines(const vk::DescriptorSetLayout& descriptorSetLayout, const vk::SampleCountFlagBits& sampleCounts, const std::vector<vk::PipelineShaderStageCreateInfo>& shaderStageInfos, const vk::PipelineCache& pipelineCache)
{
	static constexpr auto BLEND_MODE_COUNT = static_cast<size_t>(BlendMode::End);
	vk::PipelineColorBlendAttachmentState blend_modes[BLEND_MODE_COUNT] = {};

	auto push_constant_range = vk::PushConstantRange();
	push_constant_range.offset = 0;
	push_constant_range.size = static_cast<uint32_t>(sizeof(PushConstant));
	push_constant_range.stageFlags = vk::ShaderStageFlagBits::eFragment;

	auto layout_info = vk::PipelineLayoutCreateInfo();
	layout_info.pPushConstantRanges = &push_constant_range;
	layout_info.pSetLayouts = &descriptorSetLayout;
	layout_info.pushConstantRangeCount = 1;
	layout_info.setLayoutCount = 1;
	m_pipelineLayout = m_logicalDevice.createPipelineLayout(layout_info);

	vk::BlendOp alpha_blend_op[BLEND_MODE_COUNT] = {
		vk::BlendOp::eAdd, vk::BlendOp::eAdd, vk::BlendOp::eAdd,
		vk::BlendOp::eAdd, vk::BlendOp::eAdd, vk::BlendOp::eAdd,
		vk::BlendOp::eMax, vk::BlendOp::eMin, vk::BlendOp::eAdd
	};

	vk::Bool32 blend_enable[BLEND_MODE_COUNT] = {
		VK_FALSE, VK_TRUE, VK_TRUE,
		VK_TRUE, VK_TRUE, VK_TRUE,
		VK_TRUE, VK_TRUE, VK_TRUE
	};

	vk::BlendOp color_blend_op[BLEND_MODE_COUNT] = {
		vk::BlendOp::eAdd, vk::BlendOp::eAdd, vk::BlendOp::eAdd,
		vk::BlendOp::eAdd, vk::BlendOp::eAdd, vk::BlendOp::eAdd,
		vk::BlendOp::eMax, vk::BlendOp::eMin, vk::BlendOp::eAdd
	};

	vk::ColorComponentFlags color_write_mask = vk::ColorComponentFlagBits::eR
		| vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eA;

	vk::ColorComponentFlags color_write_masks[BLEND_MODE_COUNT];
	std::fill(std::begin(color_write_masks), std::end(color_write_masks),
		color_write_mask);

	vk::BlendFactor dst_alpha_blend_factor[BLEND_MODE_COUNT] = {
		vk::BlendFactor::eZero, vk::BlendFactor::eOneMinusSrcAlpha,
		vk::BlendFactor::eOne, vk::BlendFactor::eOne,
		vk::BlendFactor::eZero, vk::BlendFactor::eZero,
		vk::BlendFactor::eOne, vk::BlendFactor::eOne,
		vk::BlendFactor::eOneMinusSrcAlpha
	};

	vk::BlendFactor dst_color_blend_factor[BLEND_MODE_COUNT] = {
		vk::BlendFactor::eZero, vk::BlendFactor::eOneMinusSrcAlpha,
		vk::BlendFactor::eOne, vk::BlendFactor::eOneMinusSrcColor,
		vk::BlendFactor::eZero, vk::BlendFactor::eZero,
		vk::BlendFactor::eOne, vk::BlendFactor::eOne,
		vk::BlendFactor::eOneMinusSrcColor
	};

	vk::BlendFactor src_alpha_blend_factor[BLEND_MODE_COUNT] = {
		vk::BlendFactor::eOne, vk::BlendFactor::eOne, vk::BlendFactor::eZero,
		vk::BlendFactor::eZero, vk::BlendFactor::eOne, vk::BlendFactor::eDstAlpha,
		vk::BlendFactor::eOne, vk::BlendFactor::eOne, vk::BlendFactor::eOne
	};

	vk::BlendFactor src_color_blend_factor[BLEND_MODE_COUNT] = {
		vk::BlendFactor::eOne, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eSrcAlpha,
		vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eDstColor,
		vk::BlendFactor::eOne, vk::BlendFactor::eOne, vk::BlendFactor::eSrcAlpha
	};

	m_graphicsPipelines.resize(BLEND_MODE_COUNT);
	std::future<void> worker_threads[BLEND_MODE_COUNT];

	for (size_t i = 0; i < BLEND_MODE_COUNT; ++i)
	{
		auto color_attachment = vk::PipelineColorBlendAttachmentState();
		color_attachment.alphaBlendOp = alpha_blend_op[i];
		color_attachment.blendEnable = blend_enable[i];
		color_attachment.colorBlendOp = color_blend_op[i];
		color_attachment.colorWriteMask = color_write_masks[i];
		color_attachment.dstAlphaBlendFactor = dst_alpha_blend_factor[i];
		color_attachment.dstColorBlendFactor = dst_color_blend_factor[i];
		color_attachment.srcAlphaBlendFactor = src_alpha_blend_factor[i];
		color_attachment.srcColorBlendFactor = src_color_blend_factor[i];

		worker_threads[i] = std::async(std::launch::async, &Pipeline::CreateGraphicPipeline, this, m_logicalDevice, color_attachment, sampleCounts, shaderStageInfos, m_pipelineLayout, pipelineCache, i, m_renderPass, &m_graphicsPipelines[i]);
	}

	for (auto& thread : worker_threads)
		thread.wait();
}

void GLVK::VK::Pipeline::CreateComputePipeline()
{
}
