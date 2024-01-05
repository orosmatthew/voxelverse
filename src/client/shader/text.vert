#version 460

layout (set = 0, binding = 0) uniform GlobalUniform {
    mat4 view;
    mat4 proj;
} global_ubo;

layout (set = 1, binding = 0) uniform GlyphUniform {
    mat4 model;
    vec3 text_color;
} glyph_ubo;

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_tex_coord;

layout (location = 0) out vec2 frag_tex_coord;
layout (location = 1) out vec3 frag_text_color;

void main() {
    gl_Position = global_ubo.proj * global_ubo.view * glyph_ubo.model * vec4(in_pos, 1.0);
    frag_tex_coord = in_tex_coord;
    frag_text_color = glyph_ubo.text_color;
}