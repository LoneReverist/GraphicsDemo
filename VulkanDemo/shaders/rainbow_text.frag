#version 450 core

layout(set = 0, binding = 0) uniform sampler2D msdf_texture;
layout(push_constant) uniform PushConstants {
    vec4 bg_color;
    float screen_px_range;
    float time;
    float rainbow_width;
    float slant_factor;
} obj_data;

// origin_upper_left ensures the rainbow effect is slanted in the same direction as in Vulkan
layout(origin_upper_left) in vec4 gl_FragCoord;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_frag_color;

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

vec3 hsv_to_rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec3 msd = texture(msdf_texture, in_uv).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screen_px_dist = obj_data.screen_px_range * (sd - 0.5);
    float opacity = clamp(screen_px_dist + 0.5, 0.0, 1.0);

    // Use both x and y screen position for a slanted rainbow effect
    float hue = mod(((gl_FragCoord.x + gl_FragCoord.y * obj_data.slant_factor) / obj_data.rainbow_width) + obj_data.time * 0.2, 1.0);
    vec3 rainbow = hsv_to_rgb(vec3(hue, 1.0, 1.0));
    vec4 rainbow_color = vec4(rainbow, 1.0);

    out_frag_color = mix(obj_data.bg_color, rainbow_color, opacity);
}
