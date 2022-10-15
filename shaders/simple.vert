#version 450

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(in_pos, 0.0, 1.0);
    fragColor = in_color;
}