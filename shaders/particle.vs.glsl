#version 330

// base attributes (from ColoredVertex)
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_base_color;
// application data
layout (location = 2) in vec3 in_trans1; // transform matrix row 1
layout (location = 3) in vec3 in_trans2; // transform matrix row 2
layout (location = 4) in vec3 in_trans3; // transform matrix row 3
layout (location = 5) in vec4 in_color;  // input color

out vec4 vcolor; // output color

void main() {
    vcolor = in_color * vec4(in_base_color, 1.0);
	mat3 trans = mat3(in_trans1, in_trans2, in_trans3);

    vec3 pos = trans * vec3(in_position.xy, 1.0);
    gl_Position = vec4(pos.xy, in_position.z, 1.0);
}
