#pragma once
#include <vulkan/vulkan.hpp>

namespace GLVK
{
	namespace VK
	{
		class Shader
		{
		public:
			Shader(const vk::ShaderStageFlagBits& shaderStage, std::string_view fileName, const vk::Device& device);
			virtual ~Shader();

			const vk::PipelineShaderStageCreateInfo& GetShaderStageInfo() const noexcept
			{
				return m_shaderStageInfo;
			}

		private:
			vk::ShaderModule m_shader = nullptr;
			vk::PipelineShaderStageCreateInfo m_shaderStageInfo = {};
			vk::Device m_logicalDevice = nullptr;
		};
	}
}

