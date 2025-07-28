// Scene.ixx

module;

#include <filesystem>
#include <memory>

export module Scene;

import Camera;
import ColorPipeline;
import FontAtlas;
import GraphicsApi;
import Input;
import LightsManager;
import LightSourcePipeline;
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
	explicit Scene(GraphicsApi const & graphics_api)
		: m_graphics_api{ graphics_api }
		, m_renderer{ graphics_api }
		, m_camera{ false /*flip_proj_y*/ }
	{
	}

	void Init();
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

private:
	GraphicsApi const & m_graphics_api;
	std::filesystem::path m_resources_path;

	Renderer m_renderer;
	Camera m_camera;

	std::unique_ptr<Texture> m_ground_tex;
	std::unique_ptr<Texture> m_skybox_tex;
	std::unique_ptr<FontAtlas> m_arial_font;

	std::shared_ptr<RenderObject> m_sword0;
	std::shared_ptr<RenderObject> m_sword1;
	std::shared_ptr<RenderObject> m_red_gem;
	std::shared_ptr<RenderObject> m_green_gem;
	std::shared_ptr<RenderObject> m_blue_gem;
	std::shared_ptr<RenderObject> m_ground;
	std::shared_ptr<RenderObject> m_skybox;
	std::shared_ptr<RenderObject> m_text;

	LightsManager m_lights;

	std::unique_ptr<TextMesh> m_fps_label;

	float m_timer = 0.0;
	float m_frame_timer = 0.0;
	int m_frame_count = 0;
};
