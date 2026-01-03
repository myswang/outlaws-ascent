#version 330
// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE


in vec3 in_position;

out vec2 texcoord;

void main()
{
    gl_Position = vec4(in_position.xy, 0, 1.0);
	texcoord = (in_position.xy + 1) / 2.f;
}
