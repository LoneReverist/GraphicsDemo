// PipelineBuilder.ixx

module;

#include <filesystem>

#include <vulkan/vulkan.h>

export module PipelineBuilder;

import GraphicsApi;
import GraphicsPipeline;
import RenderObject;
import Texture;
import Vertex;

export class PipelineBuilder
{
public:
	using PerFrameConstantsCallback = GraphicsPipeline::PerFrameConstantsCallback;
	using PerObjectConstantsCallback = GraphicsPipeline::PerObjectConstantsCallback;

	PipelineBuilder(GraphicsApi const & graphics_api);
	~PipelineBuilder();

	void LoadShaders(std::filesystem::path const & vs_path, std::filesystem::path const & fs_path);

	template <IsVertex Vert>
	void SetVertexType();

	template <typename VSConstantData, typename FSConstantData = std::nullopt_t>
	void SetPushConstantTypes();

	template <typename... UniformTypes>
	void SetVSUniformTypes();

	template <typename... UniformTypes>
	void SetFSUniformTypes();

	void SetTexture(Texture const & texture) { m_texture = &texture; }
	void EnableDepthTest(bool enable) { m_enable_depth_test = enable; }

	void SetPerFrameConstantsCallback(PerFrameConstantsCallback callback) { m_per_frame_constants_callback = callback; }
	void SetPerObjectConstantsCallback(PerObjectConstantsCallback callback) { m_per_object_constants_callback = callback; }

	std::unique_ptr<GraphicsPipeline> CreatePipeline() const;

private:
	GraphicsApi const & m_graphics_api;

	VkShaderModule m_vert_shader_module{ VK_NULL_HANDLE };
	VkShaderModule m_frag_shader_module{ VK_NULL_HANDLE };

	VkVertexInputBindingDescription m_vert_binding_desc;
	std::vector<VkVertexInputAttributeDescription> m_vert_attrib_descs;

	std::vector<VkPushConstantRange> m_push_constants_ranges;
	std::vector<VkDeviceSize> m_vs_uniform_sizes;
	std::vector<VkDeviceSize> m_fs_uniform_sizes;
	Texture const * m_texture{ nullptr };

	bool m_enable_depth_test{ true };

	PerFrameConstantsCallback m_per_frame_constants_callback;
	PerObjectConstantsCallback m_per_object_constants_callback;
};

template <IsVertex Vert>
void PipelineBuilder::SetVertexType()
{
	m_vert_binding_desc = Vertex::GetBindingDesc<Vert>();
	m_vert_attrib_descs = Vertex::GetAttribDescs<Vert>();
}

template <typename VSConstantData, typename FSConstantData /*= std::nullopt_t*/>
void PipelineBuilder::SetPushConstantTypes()
{
	m_push_constants_ranges.emplace_back(
		VkPushConstantRange{
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.offset = 0,
			.size = static_cast<uint32_t>(sizeof(VSConstantData)),
		});

	if constexpr (!std::same_as<FSConstantData, std::nullopt_t>)
	{
		m_push_constants_ranges.emplace_back(
			VkPushConstantRange{
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.offset = static_cast<uint32_t>(sizeof(VSConstantData)),
				.size = static_cast<uint32_t>(sizeof(FSConstantData)),
			});
	}
}

template <typename... UniformTypes>
void PipelineBuilder::SetVSUniformTypes()
{
	m_vs_uniform_sizes = {
		static_cast<VkDeviceSize>(sizeof(UniformTypes))...
	};
}

template <typename... UniformTypes>
void PipelineBuilder::SetFSUniformTypes()
{
	m_fs_uniform_sizes = {
		static_cast<VkDeviceSize>(sizeof(UniformTypes))...
	};
}
