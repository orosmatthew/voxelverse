#version 460

layout (set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec2 in_tex_coord;

layout (location = 0) out vec3 frag_color;
layout (location = 1) out vec2 frag_tex_coord;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_pos, 1.0);
    frag_color = in_color;
    frag_tex_coord = in_tex_coord;
}