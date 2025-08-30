#version 450

layout(binding = 1) uniform samplerCube cube_map_sampler;

layout(location = 0) in vec3 in_tex_coord;

layout(location = 0) out vec4 out_frag_color;

void main()
{
	gl_FragDepth = 1.0;
	out_frag_color = texture(cube_map_sampler, in_tex_coord);
}
