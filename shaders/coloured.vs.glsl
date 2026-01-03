#version 330

// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE

// Input attributes
in vec3 in_color;
in vec3 in_position;

out vec3 vcolor;

// Application data
uniform mat3 view;
uniform mat3 projection;
uniform mat3 model;

void main()
{
	vcolor = in_color;
	vec3 pos = projection * view * model * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}