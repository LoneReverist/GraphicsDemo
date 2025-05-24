#version 330 core

layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec3 vert_normal;

uniform mat4 model_transform;
uniform mat4 view_transform;
uniform mat4 proj_transform;

out vec3 pos_world;
out vec3 normal_world;

void main()
{
	vec4 pos_world_vec4 = model_transform * vec4(vert_pos, 1.0);

	pos_world = vec3(pos_world_vec4);
	normal_world = vec3(model_transform * vec4(vert_normal, 0.0));

	gl_Position = proj_transform * view_transform * pos_world_vec4;
}
