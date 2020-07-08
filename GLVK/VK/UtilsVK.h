#pragma once
#include <glm/glm.hpp>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace GLVK
{
	namespace VK
	{
		struct QueueIndices
		{
			std::optional<uint32_t> GraphicsQueue;
			std::optional<uint32_t> PresentQueue;
			
			constexpr bool IsCompleted() const noexcept
			{
				return GraphicsQueue.has_value() && PresentQueue.has_value();
			}
		};

		struct SwapchainDetails
        {
		    vk::SurfaceCapabilitiesKHR SurfaceCapabilities;
		    std::vector<vk::SurfaceFormatKHR> Formats;
		    std::vector<vk::PresentModeKHR> PresentModes;
        };

		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec2 TexCoord;

			static std::vector<vk::VertexInputAttributeDescription> GetVertexInputAttributeDescription(uint32_t binding) noexcept
			{
				auto descs = std::vector<vk::VertexInputAttributeDescription>(3);

				descs[0] = vk::VertexInputAttributeDescription();
				descs[0].binding = binding;
				descs[0].format = vk::Format::eR32G32B32Sfloat;
				descs[0].location = 0;
				descs[0].offset = offsetof(Vertex, Position);

				descs[1] = vk::VertexInputAttributeDescription();
				descs[1].binding = binding;
				descs[1].format = vk::Format::eR32G32B32Sfloat;
				descs[1].location = 1;
				descs[1].offset = offsetof(Vertex, Normal);

				descs[2] = vk::VertexInputAttributeDescription();
				descs[2].binding = binding;
				descs[2].format = vk::Format::eR32G32Sfloat;
				descs[2].location = 2;
				descs[2].offset = offsetof(Vertex, TexCoord);

				return descs;
			}

			static vk::VertexInputBindingDescription GetVertexInputBindingDescription(uint32_t binding, const vk::VertexInputRate& inputRate) noexcept
			{
				auto desc = vk::VertexInputBindingDescription();
				desc.binding = binding;
				desc.inputRate = inputRate;
				desc.stride = static_cast<uint32_t>(sizeof(Vertex));

				return desc;
			}
		};

		inline void ThrowIfFailed(VkResult result, std::string_view message)
		{
			if (result != VK_SUCCESS)
			{
				throw std::runtime_error(message.data());
			}
		}

		inline void ThrowIfFailed(vk::Result result, std::string_view message)
		{
			if (result != vk::Result::eSuccess)
			{
				throw std::runtime_error(message.data());
			}
		}
	}
}