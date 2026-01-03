#version 330 core
/* simpleGL freetype font vertex shader */
// THIS FILE IS TAKEN FROM SIMPLEGL-3

layout (location = 0) in vec4 vertex;	// vec4 = vec2 pos (xy) + vec2 tex (zw)
out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 transform;
uniform mat4 view;

void main()
{
	gl_Position = projection * view * transform * vec4(vertex.xy, 0.0, 1.0);
	TexCoords = vertex.zw;
}