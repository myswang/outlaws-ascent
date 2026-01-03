#version 330
// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE


// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec4 fcolor;

uniform float row;
uniform float col;
uniform float num_rows;
uniform float num_cols;
uniform float tile_x;
uniform float tile_y;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
    float uv_x = (texcoord.x + col) * (1.0 / num_cols);
    float uv_y = (texcoord.y + row) * (1.0 / num_rows);
    uv_x *= tile_x;
    uv_y *= tile_y;
	color = fcolor * texture(sampler0, vec2(uv_x, uv_y));
}
