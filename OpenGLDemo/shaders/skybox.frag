#version 450 core

uniform samplerCube cube_map;

in vec3 tex_coords;

out vec4 FragColor;

void main()
{
	gl_FragDepth = 1.0;
	FragColor = texture(cube_map, tex_coords);
}
