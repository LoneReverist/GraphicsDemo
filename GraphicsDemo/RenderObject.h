// RenderObject.h

#pragma once

class ShaderProgram;

class RenderObject
{
private:
	struct Vertex {
		float m_pos[3];
		//float m_normal[3];
		float m_color[3];
	};

public:
	~RenderObject();

	void Init();
	void Render() const;

	void SetShaderProgram(std::shared_ptr<ShaderProgram> shader_program) { m_shader_program = shader_program; }
	void SetDrawWireframe(bool wireframe = true) { m_draw_wireframe = wireframe; }

private:
	void load_mesh();

private:
	std::vector<Vertex> m_verts;
	std::vector<unsigned int> m_indices;

	unsigned int m_vbo_id{ 0 }; // vertex buffer object
	unsigned int m_ebo_id{ 0 }; // element buffer object
	unsigned int m_vao_id{ 0 }; // vertex array object

	std::shared_ptr<ShaderProgram> m_shader_program;

	bool m_draw_wireframe{ false };
};
