#version 420 core

uniform sampler2D msdf_texture;
uniform float screen_px_range;
uniform vec4 bg_color;
uniform float time; // Animation time

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
    float screen_px_dist = screen_px_range * (sd - 0.5);
    float opacity = clamp(screen_px_dist + 0.5, 0.0, 1.0);

    // Use both x and y screen position for a slanted rainbow effect
    vec2 frag_pos = gl_FragCoord.xy;
    float rainbow_width = 400.0; // Adjust for rainbow width
    float slant_factor = 1.0;    // Adjust for slant angle (0.0 = horizontal, 1.0 = 45 degrees)
    float hue = mod(((frag_pos.x + frag_pos.y * slant_factor) / rainbow_width) + time * 0.2, 1.0);
    vec3 rainbow = hsv_to_rgb(vec3(hue, 1.0, 1.0));
    vec4 rainbow_color = vec4(rainbow, 1.0);

    out_frag_color = mix(bg_color, rainbow_color, opacity);
}
