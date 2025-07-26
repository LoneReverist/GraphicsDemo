#version 420 core

layout(push_constant) uniform ObjectData {
	float screen_px_range;
	vec4 bg_color;
	vec4 text_color;
} obj_data;

layout(binding = 0) uniform sampler2D msdf_texture;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_frag_color;

float median(float r, float g, float b)
{
	return max(min(r, g), min(max(r, g), b));
}

void main()
{
	vec3 msd = texture(msdf_texture, in_uv).rgb;
	float sd = median(msd.r, msd.g, msd.b);
	float screen_px_dist = obj_data.screen_px_range * (sd - 0.5);
	float opacity = clamp(screen_px_dist + 0.5, 0.0, 1.0);

	out_frag_color = mix(obj_data.bg_color, obj_data.text_color, opacity);
}
