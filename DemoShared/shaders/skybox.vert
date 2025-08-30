#version 450

layout(std140, binding = 0) uniform ViewProjUniform {
	mat4 view;
	mat4 proj;
} transforms;

layout(location = 0) in vec3 in_pos;

layout(location = 0) out vec3 out_tex_coord;

// returns a 90 degree x-axis rotation matrix
mat3 get_z_correction_matrix()
{
	float s = sin(radians(90.0));
	float c = cos(radians(90.0));
	return mat3(
		1, 0, 0,
		0, c, -s,
		0, s, c
	);
}

void main()
{
	// cubemaps expect the Y-axis to be the up direction but we want the Z-axis to be up, so rotate the texture coordinate
	out_tex_coord = get_z_correction_matrix() * in_pos;

	mat4 view = transforms.view;
	view[3][0] = 0.0;
	view[3][1] = 0.0;
	view[3][2] = 0.0;

	gl_Position = transforms.proj * view * vec4(in_pos, 1.0);
}
