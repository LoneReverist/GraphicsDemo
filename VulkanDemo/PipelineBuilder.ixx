// PipelineBuilder.ixx

module;

#include <filesystem>

#include <vulkan/vulkan.h>

export module PipelineBuilder;

import GraphicsApi;
import GraphicsPipeline;
import RenderObject;
import Mesh;

export class PipelineBuilder
{
public:
	using PerFrameConstantsCallback = GraphicsPipeline::PerFrameConstantsCallback;
	using PerObjectConstantsCallback = GraphicsPipeline::PerObjectConstantsCallback;

	PipelineBuilder(GraphicsApi const & graphics_api);
	~PipelineBuilder();

	void LoadShaders(std::filesystem::path const & vs_path, std::filesystem::path const & fs_path);

	template <typename Vertex>
	void SetVertexType();

	template <typename VSConstantData, typename FSContantData>
	void SetPushConstantTypes();

	template <typename UniformData>
	void SetUniformType();

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
	VkDeviceSize m_uniform_size{ 0 };

	PerFrameConstantsCallback m_per_frame_constants_callback;
	PerObjectConstantsCallback m_per_object_constants_callback;
};

template <typename VertexType>
void PipelineBuilder::SetVertexType()
{
	m_vert_binding_desc = Vertex::GetBindingDesc<VertexType>();
	m_vert_attrib_descs = Vertex::GetAttribDescs<NormalVertex>();
}

template <typename VSConstantData, typename FSContantData>
void PipelineBuilder::SetPushConstantTypes()
{
	m_push_constants_ranges = {
		VkPushConstantRange{
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.offset = 0,
			.size = static_cast<uint32_t>(sizeof(VSConstantData)),
		},
		VkPushConstantRange{
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.offset = static_cast<uint32_t>(sizeof(VSConstantData)),
			.size = static_cast<uint32_t>(sizeof(FSContantData)),
		}
	};
}

template <typename UniformData>
void PipelineBuilder::SetUniformType()
{
	m_uniform_size = static_cast<VkDeviceSize>(sizeof(UniformData));
}
