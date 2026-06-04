// Renderer.cpp

module;

#include <vulkan/vulkan_raii.hpp>

#include <glm/vec4.hpp>

module Dreamhearth;

import :Renderer;

namespace Dreamhearth
{
	Renderer::Renderer(RenderContext const & render_context)
		: m_render_context(render_context)
	{
	}

	void Renderer::SetViewport(int x, int y, int width, int height)
	{
		m_viewport = vk::Rect2D(vk::Offset2D(x, y), vk::Extent2D(width, height));
	}

	void transition_image_layout(
		vk::raii::CommandBuffer const & commandBuffer,
		vk::Image const & image,
		vk::ImageLayout old_layout,
		vk::ImageLayout new_layout,
		vk::AccessFlags2 src_access_mask,
		vk::AccessFlags2 dst_access_mask,
		vk::PipelineStageFlags2 src_stage_mask,
		vk::PipelineStageFlags2 dst_stage_mask,
		vk::ImageAspectFlags image_aspect_flags)
	{
		vk::ImageMemoryBarrier2 barrier = {
			.srcStageMask = src_stage_mask,
			.srcAccessMask = src_access_mask,
			.dstStageMask = dst_stage_mask,
			.dstAccessMask = dst_access_mask,
			.oldLayout = old_layout,
			.newLayout = new_layout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				.aspectMask = image_aspect_flags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		vk::DependencyInfo dependency_info = {
			.dependencyFlags = {},
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier
		};

		commandBuffer.pipelineBarrier2(dependency_info);
	}

	void Renderer::BeginDraw() const
	{
		vk::raii::CommandBuffer const & command_buffer = m_render_context.GetCurCommandBuffer();

		transition_image_layout(
			command_buffer,
			m_render_context.GetCurSwapChainImage(),
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			{},                                                        // src_access_mask (no need to wait for previous operations)
			vk::AccessFlagBits2::eColorAttachmentWrite,                // dst_access_mask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // src_stage_mask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // dst_stage_mask
			vk::ImageAspectFlagBits::eColor);

		transition_image_layout(
			command_buffer,
			*m_render_context.GetDepthImage(),
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthAttachmentOptimal,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::ImageAspectFlagBits::eDepth);

		vk::RenderingAttachmentInfo color_attachment = {
			.imageView = m_render_context.GetCurSwapChainImageView(),
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = vk::ClearColorValue(m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0f)
		};

		vk::RenderingAttachmentInfo depth_attachment = {
			.imageView = m_render_context.GetDepthImageView(),
			.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eDontCare,
			.clearValue = vk::ClearDepthStencilValue(1.0f, 0)
		};

		vk::Extent2D swap_chain_extent = m_render_context.GetSwapChainExtent();
		vk::RenderingInfo rendering_info = {
			.renderArea = { .offset = { 0, 0 }, .extent = swap_chain_extent },
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &color_attachment,
			.pDepthAttachment = &depth_attachment
		};

		command_buffer.beginRendering(rendering_info);

		// if viewport is not set, default to entire framebuffer
		vk::Rect2D viewport = m_viewport;
		if (viewport.extent.width == 0 || viewport.extent.height == 0)
			viewport = vk::Rect2D(vk::Offset2D(0, 0), swap_chain_extent);
		vk::Rect2D scissor{
			.offset = { std::max(0, viewport.offset.x), std::max(0, viewport.offset.y) },
			.extent = { std::min(static_cast<uint32_t>(viewport.extent.width), swap_chain_extent.width),
				 std::min(static_cast<uint32_t>(viewport.extent.height), swap_chain_extent.height) }
		};

		command_buffer.setViewport(0, vk::Viewport{
			static_cast<float>(viewport.offset.x), static_cast<float>(viewport.offset.y),
			static_cast<float>(viewport.extent.width), static_cast<float>(viewport.extent.height),
			0.0f, 1.0f });
		command_buffer.setScissor(0, scissor);
	}

	void Renderer::EndDraw() const
	{
		vk::raii::CommandBuffer const & command_buffer = m_render_context.GetCurCommandBuffer();

		command_buffer.endRendering();

		transition_image_layout(
			command_buffer,
			m_render_context.GetCurSwapChainImage(),
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits2::eColorAttachmentWrite,             // src_access_mask
			{},                                                     // dst_access_mask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // src_stage_mask
			vk::PipelineStageFlagBits2::eBottomOfPipe,              // dst_stage_mask
			vk::ImageAspectFlagBits::eColor);

		command_buffer.end();
	}

	void Renderer::BeginTextureDraw(Texture const & target, glm::vec4 const & clear_color) const
	{
		vk::raii::CommandBuffer const & command_buffer = m_render_context.GetCurCommandBuffer();

		transition_image_layout(
			command_buffer,
			target.GetImage(),
			target.GetLayout(),
			vk::ImageLayout::eColorAttachmentOptimal,
			target.GetLayout() == vk::ImageLayout::eShaderReadOnlyOptimal ? vk::AccessFlagBits2::eShaderSampledRead : vk::AccessFlags2{},
			vk::AccessFlagBits2::eColorAttachmentWrite,
			target.GetLayout() == vk::ImageLayout::eShaderReadOnlyOptimal ? vk::PipelineStageFlagBits2::eFragmentShader : vk::PipelineStageFlagBits2::eTopOfPipe,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::ImageAspectFlagBits::eColor);
		target.SetLayout(vk::ImageLayout::eColorAttachmentOptimal);

		vk::RenderingAttachmentInfo color_attachment = {
			.imageView = target.GetImageView(),
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = vk::ClearColorValue(clear_color.r, clear_color.g, clear_color.b, clear_color.a)
		};

		vk::RenderingInfo rendering_info = {
			.renderArea = { .offset = { 0, 0 }, .extent = { target.GetWidth(), target.GetHeight() } },
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &color_attachment,
		};

		command_buffer.beginRendering(rendering_info);

		vk::Rect2D viewport{ { 0, 0 }, { target.GetWidth(), target.GetHeight() } };
		command_buffer.setViewport(0, vk::Viewport{
			0.0f, 0.0f,
			static_cast<float>(target.GetWidth()), static_cast<float>(target.GetHeight()),
			0.0f, 1.0f });
		command_buffer.setScissor(0, viewport);
	}

	void Renderer::EndTextureDraw(Texture const & target) const
	{
		vk::raii::CommandBuffer const & command_buffer = m_render_context.GetCurCommandBuffer();

		command_buffer.endRendering();

		transition_image_layout(
			command_buffer,
			target.GetImage(),
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::AccessFlagBits2::eShaderSampledRead,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::ImageAspectFlagBits::eColor);
		target.SetLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	}
} // namespace Dreamhearth
