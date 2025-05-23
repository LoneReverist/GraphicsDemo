#version 330 core

layout (location = 0) in vec3 vert_pos;

uniform mat4 view_transform;
uniform mat4 proj_transform;

out vec3 tex_coords;

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
	// opengl cubemaps expect the Y-axis to be the up direction but we want the Z-axis to be up, so rotate the texture coordinate
	tex_coords = get_z_correction_matrix() * vert_pos;

	mat4 view = view_transform;
	view[3][0] = 0.0;
	view[3][1] = 0.0;
	view[3][2] = 0.0;

	gl_Position = proj_transform * view * vec4(vert_pos, 1.0);
}
