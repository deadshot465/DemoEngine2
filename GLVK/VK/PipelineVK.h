#pragma once
#include <mutex>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "../../UtilsCommon.h"

namespace GLVK
{
	namespace VK
	{
		class Pipeline
		{
		public:
			explicit Pipeline(const vk::Device& device);
			~Pipeline();

            void CreateGraphicPipeline(const vk::Device& device, const vk::PipelineColorBlendAttachmentState& colorBlendAttachment, const vk::SampleCountFlagBits& sampleCounts, const std::vector<vk::PipelineShaderStageCreateInfo>& shaderStageInfos, const vk::PipelineLayout& pipelineLayout, const vk::PipelineCache& pipelineCache, size_t blendModeIndex, const vk::RenderPass& renderPass, vk::Pipeline* pipeline);
			void CreateRenderPass(const vk::Format& graphicsFormat, const vk::Format& depthFormat, const vk::SampleCountFlagBits& sampleCount);
			void CreateGraphicPipelines(const vk::DescriptorSetLayout& descriptorSetLayout, const vk::SampleCountFlagBits& sampleCounts, const std::vector<vk::PipelineShaderStageCreateInfo>& shaderStageInfos, const vk::PipelineCache& pipelineCache = nullptr, const ShaderType& shaderType = ShaderType::BasicShader);
			void CreateComputePipeline();

			[[nodiscard]] const vk::RenderPass& GetRenderPass() const noexcept
			{
				return m_renderPass;
			}

			[[nodiscard]] const vk::Pipeline& GetPipeline(const BlendMode& blendMode, const ShaderType& shaderType) const noexcept
            {
				return m_graphicsPipelines.at(shaderType)[size_t(blendMode)];
            }

			[[nodiscard]] const vk::PipelineLayout& GetPipelineLayout(const ShaderType& shaderType) const noexcept
            {
			    return m_pipelineLayouts.at(shaderType);
            }

		private:
			inline static std::mutex m_mutex = std::mutex();

			vk::RenderPass m_renderPass = nullptr;
			std::unordered_map<ShaderType, vk::PipelineLayout> m_pipelineLayouts;
			//std::vector<vk::Pipeline> m_graphicsPipelines;
			std::unordered_map<ShaderType, std::vector<vk::Pipeline>> m_graphicsPipelines;
			vk::Device m_logicalDevice = nullptr;
			bool m_ownedRenderPass = false;
		};
	}
}

