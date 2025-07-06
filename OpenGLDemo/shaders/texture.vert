#version 420 core

uniform mat4 model_transform;

layout(std140, binding = 0) uniform ViewProjUniform {
	mat4 view;
	mat4 proj;
} transforms;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_tex_coord;

layout(location = 0) out vec3 out_pos_world;
layout(location = 1) out vec3 out_normal_world;
layout(location = 2) out vec2 out_tex_coord;

void main()
{
	vec4 pos_world_vec4 = model_transform * vec4(in_pos, 1.0);

	out_pos_world = vec3(pos_world_vec4);
	out_normal_world = vec3(model_transform * vec4(in_normal, 0.0));
	out_tex_coord = in_tex_coord;

	gl_Position = transforms.proj * transforms.view * pos_world_vec4;
}
