// PipelineBuilder.ixx

module;

#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

export module Dreamhearth:PipelineBuilder;

import :RenderContext;
import :GraphicsError;
import :Pipeline;
import :Texture;
import :VertexLayout;

namespace Dreamhearth
{
	export class PipelineBuilder
	{
	public:
		using PerFrameConstantsCallback = Pipeline::PerFrameConstantsCallback;
		using PerObjectConstantsCallback = Pipeline::PerObjectConstantsCallback;

		explicit PipelineBuilder(RenderContext const & render_context);

		std::expected<void, GraphicsError> LoadShaders(std::filesystem::path const & vs_path, std::filesystem::path const & fs_path);

		template <VertexWithLayout VertexT>
		void SetVertexType();

		template <typename InstanceDataT>
		void SetInstanceType();

		template <typename ObjectDataVS = std::nullopt_t, typename ObjectDataFS = std::nullopt_t>
		void SetObjectDataTypes();

		template <typename... UniformTypes>
		void SetVSUniformTypes();

		template <typename... UniformTypes>
		void SetFSUniformTypes();

		void SetHasTexture(bool has_texture) { m_has_texture = has_texture; }
		void SetDepthTestOptions(DepthTestOptions const & options) { m_depth_test_options = options; }
		void SetBlendOptions(BlendOptions const & options) { m_blend_options = options; }
		void SetCullMode(CullMode cull_mode) { m_cull_mode = cull_mode; }

		void SetPerFrameConstantsCallback(PerFrameConstantsCallback callback) { m_per_frame_constants_callback = callback; }
		void SetPerObjectConstantsCallback(PerObjectConstantsCallback callback) { m_per_object_constants_callback = callback; }

		std::expected<Pipeline, GraphicsError> CreatePipeline() const;

	private:
		RenderContext const & m_render_context;

		vk::raii::ShaderModule m_vert_shader_module = nullptr;
		vk::raii::ShaderModule m_frag_shader_module = nullptr;

		std::vector<vk::VertexInputBindingDescription> m_binding_descs;
		std::vector<vk::VertexInputAttributeDescription> m_attrib_descs;

		std::vector<vk::PushConstantRange> m_push_constants_ranges;
		std::vector<vk::DeviceSize> m_vs_uniform_sizes;
		std::vector<vk::DeviceSize> m_fs_uniform_sizes;
		bool m_has_texture = false;

		DepthTestOptions m_depth_test_options;
		BlendOptions m_blend_options;

		// PipelineBuilder does not have a default state for the cull mode, a null optional here means "not set yet".
		// PipelineBuilder requires the cull mode to be set explicitly because it is a common source of errors.
		std::optional<CullMode> m_cull_mode;

		PerFrameConstantsCallback m_per_frame_constants_callback;
		PerObjectConstantsCallback m_per_object_constants_callback;
	};

	template <VertexWithLayout VertexT>
	void PipelineBuilder::SetVertexType()
	{
		LayoutDesc layout = VertexT::CreateLayout();
		m_binding_descs.push_back(GetBindingDesc(layout));
		auto vertex_attrib_descs = GetAttribDescs(layout);
		m_attrib_descs.insert(m_attrib_descs.end(), vertex_attrib_descs.begin(), vertex_attrib_descs.end());
	}

	template <typename InstanceDataT>
	void PipelineBuilder::SetInstanceType()
	{
		LayoutDesc layout = InstanceDataT::CreateLayout();
		m_binding_descs.push_back(GetBindingDesc(layout));
		auto vertex_attrib_descs = GetAttribDescs(layout);
		m_attrib_descs.insert(m_attrib_descs.end(), vertex_attrib_descs.begin(), vertex_attrib_descs.end());
	}

	template <typename ObjectDataVS /*= std::nullopt_t*/, typename ObjectDataFS /*= std::nullopt_t*/>
	void PipelineBuilder::SetObjectDataTypes()
	{
		static_assert(!std::same_as<ObjectDataVS, std::nullopt_t> || !std::same_as<ObjectDataFS, std::nullopt_t>,
			"At least one push constant data must be provided");

		std::uint32_t offset = 0;

		if constexpr (!std::same_as<ObjectDataVS, std::nullopt_t>)
		{
			m_push_constants_ranges.emplace_back(
				vk::PushConstantRange{
					.stageFlags = vk::ShaderStageFlagBits::eVertex,
					.offset = offset,
					.size = static_cast<std::uint32_t>(sizeof(ObjectDataVS)),
				});

			offset = static_cast<std::uint32_t>(sizeof(ObjectDataVS));
		}

		if constexpr (!std::same_as<ObjectDataFS, std::nullopt_t>)
		{
			m_push_constants_ranges.emplace_back(
				vk::PushConstantRange{
					.stageFlags = vk::ShaderStageFlagBits::eFragment,
					.offset = offset,
					.size = static_cast<std::uint32_t>(sizeof(ObjectDataFS)),
				});
		}
	}

	template <typename... UniformTypes>
	void PipelineBuilder::SetVSUniformTypes()
	{
		m_vs_uniform_sizes = {
			static_cast<vk::DeviceSize>(sizeof(UniformTypes))...
		};
	}

	template <typename... UniformTypes>
	void PipelineBuilder::SetFSUniformTypes()
	{
		m_fs_uniform_sizes = {
			static_cast<vk::DeviceSize>(sizeof(UniformTypes))...
		};
	}
} // namespace Dreamhearth
