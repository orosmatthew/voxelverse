#version 460

layout (set = 0, binding = 1) uniform sampler2D world_sampler;
layout (set = 1, binding = 1) uniform sampler2D tex_sampler;

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec2 frag_tex_coord;
layout (location = 2) in vec2 frag_scale;
layout (location = 3) in float frag_contrast;
layout (location = 4) in vec2 frag_world_size;

layout (location = 0) out vec4 out_color;

float color_to_gray(vec4 color) {
    float gray = dot(color.rgb, vec3(0.3, 0.59, 0.11));
    return gray;
}

void main() {
    vec4 tex_color = texture(tex_sampler, frag_tex_coord);
    if (tex_color.a < 0.1) {
        discard;
    }
    //    if (frag_contrast > 0) {
    //        vec4 world_color = texture(world_sampler, frag_tex_coord * frag_scale + vec2(0.5 - frag_scale.x / 2.0, 0.5 - frag_scale.y / 2.0));
    //        float world_gray = color_to_gray(world_color);
    //        float inv_world_gray = 1.0 - world_gray;
    //        if (inv_world_gray < 0.5) {
    //            out_color = tex_color;
    //        } else {
    //            out_color = vec4(1, 1, 1, 0) - tex_color;
    //        }
    //    } else {
    //    out_color = vec4(frag_tex_coord.x, frag_tex_coord.y, 0, 0);
    out_color = tex_color;
    //    }

}