#version 330
// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE


// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 projection;
uniform mat3 view;
uniform mat3 model;

void main()
{
	texcoord = in_texcoord;
	vec3 pos = projection * view * model * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}