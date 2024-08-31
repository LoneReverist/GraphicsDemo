#version 330 core

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

uniform vec3 object_color;

uniform vec3 ambient_light_color;

uniform PointLight pointlight_1;
uniform PointLight pointlight_2;
uniform PointLight pointlight_3;

uniform SpotLight spotlight_1;

in vec3 pos_world;
in vec3 normal_world;

out vec4 FragColor;

vec3 ColorFromPointLight(PointLight light, vec3 normal)
{
	vec3 pos_to_light = normalize(light.pos - pos_world);
	float light_ratio = max(dot(normal, pos_to_light), 0.0f);
	float attenuation = pow(1.0f - max(0.0f, distance(light.pos, pos_world) / light.radius), 2.0f);
	return light_ratio * attenuation * light.color;
}

vec3 ColorFromSpotLight(SpotLight light, vec3 normal)
{
	vec3 pos_to_light = normalize(light.pos - pos_world);
	float surface_ratio = max(0.0f, dot(-pos_to_light, light.dir));
	float spot_factor = (surface_ratio > light.outer_radius) ? 1 : 0;
	float light_ratio = max(0.0f, dot(pos_to_light, normal));
	float attenuation = pow(1.0f - max(0.0f, (light.inner_radius - surface_ratio) / (light.inner_radius - light.outer_radius)), 2.0f);
	return light_ratio * attenuation * spot_factor * light.color;
}

void main()
{
	vec3 normal = normalize(normal_world);

	vec3 light_color = ambient_light_color;

	light_color += ColorFromPointLight(pointlight_1, normal);
	light_color += ColorFromPointLight(pointlight_2, normal);
	light_color += ColorFromPointLight(pointlight_3, normal);

	light_color += ColorFromSpotLight(spotlight_1, normal);

	FragColor = vec4(light_color * object_color, 1.0);
}
