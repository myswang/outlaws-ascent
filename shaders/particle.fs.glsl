#version 330

// From Vertex Shader
in vec4 vcolor;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	color = vcolor;
}