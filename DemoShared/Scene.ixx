// Scene.ixx

module;

#include <filesystem>
#include <memory>
#include <vector>

export module Scene;

import Camera;
import ColorPipeline;
import FontAtlas;
import GraphicsApi;
import Input;
import LightsManager;
import LightSourcePipeline;
import RainbowTextPipeline;
import ReflectionPipeline;
import Renderer;
import RenderObject;
import SkyboxPipeline;
import TextMesh;
import TextPipeline;
import Texture;
import TexturePipeline;

export class Scene
{
public:
	explicit Scene(GraphicsApi const & graphics_api, std::string const & title);

	void OnViewportResized(int width, int height);

	void Update(double delta_time, Input const & input);
	void Render() const;

private:
	ColorPipeline create_color_pipeline();
	TexturePipeline create_texture_pipeline(Texture const & texture);
	SkyboxPipeline create_skybox_pipeline(Texture const & texture);
	LightSourcePipeline create_light_source_pipeline();
	ReflectionPipeline create_reflection_pipeline(Texture const & texture);
	TextPipeline create_text_pipeline(FontAtlas const & font_atlas);
	RainbowTextPipeline create_rainbow_text_pipeline(FontAtlas const & font_atlas);

private:
	GraphicsApi const & m_graphics_api;
	std::filesystem::path const m_resources_path;
	std::string const m_title;

	Renderer m_renderer;
	Camera m_camera;

	std::unique_ptr<Texture> m_ground_tex;
	std::unique_ptr<Texture> m_skybox_tex;
	std::unique_ptr<FontAtlas> m_arial_font;

	std::unique_ptr<TextMesh> m_fps_mesh;
	std::unique_ptr<TextMesh> m_title_mesh;

	std::vector<std::shared_ptr<RenderObject>> m_render_objs;

	ReflectionPipeline::ObjectData m_sword0;
	ReflectionPipeline::ObjectData m_sword1;
	LightSourcePipeline::ObjectData m_red_gem;
	LightSourcePipeline::ObjectData m_green_gem;
	LightSourcePipeline::ObjectData m_blue_gem;
	TexturePipeline::ObjectData m_ground;
	TextPipeline::ObjectData m_fps_label;
	RainbowTextPipeline::ObjectData m_title_label;
	LightSourcePipeline::ObjectData m_tree;
	ColorPipeline::ObjectData m_tree_with_material;

	LightsManager m_lights;

	float m_timer = 0.0f;
	float m_frame_timer = 0.0f;
	int m_frame_count = 0;

	float m_dpi_scale = 1.0f;
};
