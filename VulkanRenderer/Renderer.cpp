// Renderer.cpp

module;

#include <vulkan/vulkan_raii.hpp>

module Renderer;

Renderer::Renderer(GraphicsApi const & graphics_api)
	: m_graphics_api(graphics_api)
{
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
	vk::raii::CommandBuffer const & command_buffer = m_graphics_api.GetCurCommandBuffer();

	command_buffer.begin({});

	transition_image_layout(
		command_buffer,
		m_graphics_api.GetCurSwapChainImage(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},                                                        // src_access_mask (no need to wait for previous operations)
		vk::AccessFlagBits2::eColorAttachmentWrite,                // dst_access_mask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // src_stage_mask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // dst_stage_mask
		vk::ImageAspectFlagBits::eColor);

	transition_image_layout(
		command_buffer,
		*m_graphics_api.GetDepthImage(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthAttachmentOptimal,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::ImageAspectFlagBits::eDepth);

	vk::RenderingAttachmentInfo attachment_info = {
		.imageView = m_graphics_api.GetCurSwapChainImageView(),
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = vk::ClearColorValue(m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0f)
	};

	vk::RenderingAttachmentInfo depth_attachment_info = {
		.imageView = m_graphics_api.GetDepthImageView(),
		.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eDontCare,
		.clearValue = vk::ClearDepthStencilValue(1.0f, 0)
	};

	vk::Extent2D swap_chain_extent = m_graphics_api.GetSwapChainExtent();
	vk::RenderingInfo rendering_info = {
		.renderArea = { .offset = { 0, 0 }, .extent = swap_chain_extent },
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachment_info,
		.pDepthAttachment = &depth_attachment_info
	};

	command_buffer.beginRendering(rendering_info);

	command_buffer.setViewport(0, vk::Viewport(0.0f, 0.0f,
		static_cast<float>(swap_chain_extent.width), static_cast<float>(swap_chain_extent.height), 0.0f, 1.0f));
	command_buffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swap_chain_extent));
}

void Renderer::EndDraw() const
{
	vk::raii::CommandBuffer const & command_buffer = m_graphics_api.GetCurCommandBuffer();

	command_buffer.endRendering();

	transition_image_layout(
		command_buffer,
		m_graphics_api.GetCurSwapChainImage(),
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,             // src_access_mask
		{},                                                     // dst_access_mask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // src_stage_mask
		vk::PipelineStageFlagBits2::eBottomOfPipe,              // dst_stage_mask
		vk::ImageAspectFlagBits::eColor);

	command_buffer.end();
}
