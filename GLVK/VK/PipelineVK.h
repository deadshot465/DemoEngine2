#pragma once
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
			void CreateGraphicPipeline();
			void CreateComputePipeline();

		private:
			vk::RenderPass m_renderPass = nullptr;
			vk::Pipeline m_graphicsPipeline = nullptr;
			vk::Device m_logicalDevice = nullptr;
			bool m_ownedRenderPass = false;
		};
	}
}

