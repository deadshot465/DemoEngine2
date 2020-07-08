#include "ShaderVK.h"
#include <string_view>
#include "../../UtilsCommon.h"

GLVK::VK::Shader::Shader(const vk::ShaderStageFlagBits& shaderStage, std::string_view fileName, const vk::Device& device)
	: m_logicalDevice(device)
{
	auto data = ReadFromFile(fileName);
	auto info = vk::ShaderModuleCreateInfo();
	info.codeSize = data.size();
	info.pCode = reinterpret_cast<const uint32_t*>(data.data());
	m_shader = device.createShaderModule(info);

	m_shaderStageInfo = vk::PipelineShaderStageCreateInfo();
	m_shaderStageInfo.module = m_shader;
	m_shaderStageInfo.pName = "main";
	m_shaderStageInfo.pSpecializationInfo = nullptr;
	m_shaderStageInfo.stage = shaderStage;
}

GLVK::VK::Shader::~Shader()
{
	m_logicalDevice.destroyShaderModule(m_shader);
}