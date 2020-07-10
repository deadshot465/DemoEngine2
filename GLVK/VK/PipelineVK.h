#pragma once
#include <mutex>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace GLVK
{
	namespace VK
	{
		class Pipeline
		{
		public:
			Pipeline(const vk::Device& device);
			~Pipeline();

			void CreateRenderPass(const vk::Format& graphicsFormat, const vk::Format& depthFormat, const vk::SampleCountFlagBits& sampleCount);
			void CreateGraphicPipelines(const vk::DescriptorSetLayout& descriptorSetLayout, const vk::SampleCountFlagBits& sampleCounts, const std::vector<vk::PipelineShaderStageCreateInfo>& shaderStageInfos, const vk::PipelineCache& pipelineCache = nullptr);
			void CreateComputePipeline();

			const vk::RenderPass& GetRenderPass() const noexcept
			{
				return m_renderPass;
			}

		private:
			static void CreateGraphicsPipeline(const vk::Device& device, const vk::PipelineColorBlendAttachmentState& colorBlendAttachment, const vk::SampleCountFlagBits& sampleCounts, const std::vector<vk::PipelineShaderStageCreateInfo>& shaderStageInfos, const vk::PipelineLayout& pipelineLayout, const vk::PipelineCache& pipelineCache, size_t blendModeIndex, const vk::RenderPass& renderPass, std::vector<vk::Pipeline>& pipelines);

			inline static std::mutex m_mutex = std::mutex();

			vk::RenderPass m_renderPass = nullptr;
			vk::PipelineLayout m_pipelineLayout = nullptr;
			std::vector<vk::Pipeline> m_graphicsPipelines;
			vk::Device m_logicalDevice = nullptr;
			bool m_ownedRenderPass = false;
		};
	}
}

