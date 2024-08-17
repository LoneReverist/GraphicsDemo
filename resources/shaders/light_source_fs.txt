#version 330 core

uniform vec3 object_color;
uniform vec3 camera_pos_world;

in vec3 pos_world;
in vec3 normal_world;

out vec4 FragColor;

void main()
{
	vec3 normal = normalize(normal_world);

	vec3 pos_to_light = normalize(camera_pos_world - pos_world);
	float light_ratio = max(dot(normal, pos_to_light), 0.0f);

	FragColor = vec4(object_color * light_ratio, 1.0);
}
