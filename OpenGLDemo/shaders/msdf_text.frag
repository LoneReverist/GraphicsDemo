#version 420 core

uniform sampler2D msdf_texture;
uniform float screen_px_range;
uniform vec4 bg_color;
uniform vec4 text_color;

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
	float screen_px_dist = screen_px_range * (sd - 0.5);
	float opacity = clamp(screen_px_dist + 0.5, 0.0, 1.0);
	
	out_frag_color = mix(bg_color, text_color, opacity);
}
