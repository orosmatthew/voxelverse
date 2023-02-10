#version 460

layout (set = 0, binding = 0) uniform GlobalUniform {
    mat4 view;
    mat4 proj;
    vec2 world_size;
} global_ubo;

layout (set = 1, binding = 0) uniform ObjectUnifom {
    mat4 model;
    vec2 scale;
    float contrast;
} object_ubo;

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec2 in_tex_coord;

layout (location = 0) out vec3 frag_color;
layout (location = 1) out vec2 frag_tex_coord;
layout (location = 2) out vec2 frag_scale;
layout (location = 3) out float frag_contrast;
layout (location = 4) out vec2 frag_world_size;

void main() {
    gl_Position = global_ubo.proj * global_ubo.view * object_ubo.model * vec4(in_pos, 1.0);
    frag_color = in_color;
    frag_tex_coord = in_tex_coord;
    frag_scale = object_ubo.scale;
    frag_contrast = object_ubo.contrast;
    frag_world_size = global_ubo.world_size;
}