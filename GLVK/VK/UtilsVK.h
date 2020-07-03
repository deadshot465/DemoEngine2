#pragma once
#include <optional>
#include <stdexcept>
#include <string_view>
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