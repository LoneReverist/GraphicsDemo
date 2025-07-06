#version 420 core

struct PointLight
{
	vec3 pos;
	vec3 color;
	float radius;
};

struct SpotLight
{
	vec3 pos;
	vec3 dir;
	vec3 color;
	float inner_radius;
	float outer_radius;
};

layout(std140, binding = 1) uniform LightsUniform {
	vec3 ambient_light_color;

	PointLight pointlight_1;
	PointLight pointlight_2;
	PointLight pointlight_3;

	SpotLight spotlight_1;
} lights;

uniform sampler2D tex_sampler;

layout(location = 0) in vec3 in_pos_world;
layout(location = 1) in vec3 in_normal_world;
layout(location = 2) in vec2 in_tex_coord;

layout(location = 0) out vec4 out_frag_color;

vec3 ColorFromPointLight(PointLight light, vec3 normal)
{
	vec3 pos_to_light = normalize(light.pos - in_pos_world);
	float light_ratio = max(dot(normal, pos_to_light), 0.0f);
	float attenuation = pow(1.0f - max(0.0f, distance(light.pos, in_pos_world) / light.radius), 2.0f);
	return light_ratio * attenuation * light.color;
}

vec3 ColorFromSpotLight(SpotLight light, vec3 normal)
{
	vec3 pos_to_light = normalize(light.pos - in_pos_world);
	float surface_ratio = max(0.0f, dot(-pos_to_light, light.dir));
	float spot_factor = (surface_ratio > light.outer_radius) ? 1 : 0;
	float light_ratio = max(0.0f, dot(pos_to_light, normal));
	float attenuation = pow(1.0f - max(0.0f, (light.inner_radius - surface_ratio) / (light.inner_radius - light.outer_radius)), 2.0f);
	return light_ratio * attenuation * spot_factor * light.color;
}

void main()
{
	vec3 normal = normalize(in_normal_world);

	vec3 light_color = lights.ambient_light_color;

	light_color += ColorFromPointLight(lights.pointlight_1, normal);
	light_color += ColorFromPointLight(lights.pointlight_2, normal);
	light_color += ColorFromPointLight(lights.pointlight_3, normal);

	light_color += ColorFromSpotLight(lights.spotlight_1, normal);

	out_frag_color = texture(tex_sampler, in_tex_coord) * vec4(light_color, 1.0);
}
