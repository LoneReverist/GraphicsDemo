// Renderer.cpp

module;

#include <cstdint>
#include <expected>
#include <string>

#include <vulkan/vulkan.h>

module Renderer;

Renderer::Renderer(GraphicsApi const & graphics_api)
	: m_graphics_api(graphics_api)
{
}

std::expected<void, GraphicsError> Renderer::BeginDraw() const
{
	VkCommandBuffer command_buffer = m_graphics_api.GetCurCommandBuffer();
	VkExtent2D sc_extent = m_graphics_api.GetSwapChainExtent();

	VkCommandBufferBeginInfo begin_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = 0,
		.pInheritanceInfo = nullptr
	};

	VkResult result = vkBeginCommandBuffer(command_buffer, &begin_info);
	if (result != VK_SUCCESS)
		return std::unexpected{ GraphicsError{ "Renderer::Render: vkBeginCommandBuffer failed with code: " + std::to_string(result) } };

	std::array<VkClearValue, 2> clear_values = {
		VkClearValue{.color{ m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0f } },
		VkClearValue{.depthStencil{ 1.0f, 0 } }
	};

	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(sc_extent.width),
		.height = static_cast<float>(sc_extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkRect2D scissor{
		.offset = { 0, 0 },
		.extent = sc_extent
	};

	VkRenderPassBeginInfo render_pass_info{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = m_graphics_api.GetRenderPass(),
		.framebuffer = m_graphics_api.GetCurFrameBuffer(),
		.renderArea = scissor,
		.clearValueCount = static_cast<std::uint32_t>(clear_values.size()),
		.pClearValues = clear_values.data()
	};

	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	return {};
}

std::expected<void, GraphicsError> Renderer::EndDraw() const
{
	VkCommandBuffer command_buffer = m_graphics_api.GetCurCommandBuffer();

	vkCmdEndRenderPass(command_buffer);

	VkResult result = vkEndCommandBuffer(command_buffer);
	if (result != VK_SUCCESS)
		return std::unexpected{ GraphicsError{ "Renderer::Render: vkEndCommandBuffer failed with code: " + std::to_string(result) } };

	return {};
}
