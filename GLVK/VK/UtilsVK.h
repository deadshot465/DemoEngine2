#pragma once
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "../../Structures/Vertex.h"

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

		struct DirectionalLight
		{
			alignas(16) glm::vec4 Diffuse;
			alignas(16) glm::vec3 LightDirection;
			alignas(4) float AmbientIntensity;
			alignas(4) float SpecularIntensity;
		};
		
		struct PushConstant
		{
			alignas(4) uint32_t TextureIndex;
			alignas(16) glm::vec4 ObjectColor;
		};

		inline std::vector<vk::VertexInputAttributeDescription> GetVertexInputAttributeDescription(uint32_t binding) noexcept
		{
			auto descs = std::vector<vk::VertexInputAttributeDescription>(3);

			descs[0] = vk::VertexInputAttributeDescription();
			descs[0].binding = binding;
			descs[0].format = vk::Format::eR32G32B32Sfloat;
			descs[0].location = 0;
			descs[0].offset = offsetof(Vertex, Vertex::Position);

			descs[1] = vk::VertexInputAttributeDescription();
			descs[1].binding = binding;
			descs[1].format = vk::Format::eR32G32B32Sfloat;
			descs[1].location = 1;
			descs[1].offset = offsetof(Vertex, Vertex::Normal);

			descs[2] = vk::VertexInputAttributeDescription();
			descs[2].binding = binding;
			descs[2].format = vk::Format::eR32G32Sfloat;
			descs[2].location = 2;
			descs[2].offset = offsetof(Vertex, Vertex::TexCoord);

			return descs;
		}

		inline vk::VertexInputBindingDescription GetVertexInputBindingDescription(uint32_t binding, const vk::VertexInputRate& inputRate) noexcept
		{
			auto desc = vk::VertexInputBindingDescription();
			desc.binding = binding;
			desc.inputRate = inputRate;
			desc.stride = static_cast<uint32_t>(sizeof(Vertex));

			return desc;
		}

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

		/// <summary>
		/// Create a single time only command buffer and begin recording.
		/// </summary>
		/// <param name="device">The logical device.</param>
		/// <param name="commandPool">The command pool.</param>
		/// <returns>A created single time only command buffer.</returns>
		inline vk::CommandBuffer CreateSingleTimeBuffer(const vk::Device& device, const vk::CommandPool& commandPool)
		{
			auto alloc_info = vk::CommandBufferAllocateInfo();
			alloc_info.commandBufferCount = 1;
			alloc_info.commandPool = commandPool;
			alloc_info.level = vk::CommandBufferLevel::ePrimary;

			auto buffer = device.allocateCommandBuffers(alloc_info)[0];

			auto begin_info = vk::CommandBufferBeginInfo();
			begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
			begin_info.pInheritanceInfo = nullptr;
			
			buffer.begin(begin_info);
			return buffer;
		}

		/// <summary>
		/// End the single time command buffer and submit with the provided queue.
		/// </summary>
		/// <param name="commandBuffer">The command buffer to execute.</param>
		/// <param name="device">The logical device.</param>
		/// <param name="commandPool">The command pool.</param>
		/// <param name="graphicsQueue">The queue used to submit the command buffer.</param>
		inline void ExecuteCommandBuffer(vk::CommandBuffer& commandBuffer, const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& graphicsQueue)
		{
			commandBuffer.end();

			auto submit_info = vk::SubmitInfo();
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &commandBuffer;
			submit_info.pSignalSemaphores = nullptr;
			submit_info.pWaitDstStageMask = nullptr;
			submit_info.pWaitSemaphores = nullptr;
			submit_info.signalSemaphoreCount = 0;
			submit_info.waitSemaphoreCount = 0;

			graphicsQueue.submit(submit_info, nullptr);
			graphicsQueue.waitIdle();
			device.freeCommandBuffers(commandPool, commandBuffer);
		}

		inline void* AllocateAlignedMemory(size_t size, size_t alignment)
		{
#ifdef _WIN32
			return _aligned_malloc(size, alignment);
#else
			void* ptr = nullptr;
			posix_memalign(&ptr, alignment, size);
			return ptr;
#endif
		}

		inline void ReleaseAlignedMemory(void* alignedMemory)
		{
#ifdef _WIN32
			_aligned_free(alignedMemory);
#endif
		}
	}
}