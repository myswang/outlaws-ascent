#version 330

// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE
// From Vertex Shader
in vec3 vcolor;

// Application data
uniform vec4 fcolor;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	color = fcolor * vec4(vcolor, 1.0);
}