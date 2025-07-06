#version 420 core

uniform vec3 object_color;

layout(std140, binding = 1) uniform CameraPosUniform {
	vec3 pos_world;
} camera;

layout(location = 0) in vec3 in_pos_world;
layout(location = 1) in vec3 in_normal_world;

layout(location = 0) out vec4 out_frag_color;

void main()
{
	vec3 normal = normalize(in_normal_world);

	vec3 pos_to_light = normalize(camera.pos_world - in_pos_world);
	float light_ratio = max(dot(normal, pos_to_light), 0.0f);

	out_frag_color = vec4(object_color * light_ratio, 1.0);
}
