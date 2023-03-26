#version 460

layout (set = 1, binding = 1) uniform sampler2D tex_sampler;

layout (location = 0) in vec2 frag_tex_coord;
layout (location = 1) in vec3 frag_text_color;

layout (location = 0) out vec4 out_color;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex_sampler, frag_tex_coord).r);
    if (sampled.a < 0.0001) {
        discard;
    }
    out_color = vec4(frag_text_color, 1.0) * sampled;
    //    out_color = vec4(frag_tex_coord.x, frag_tex_coord.y, 0.0, 1.0);
}