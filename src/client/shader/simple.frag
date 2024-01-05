#version 460

layout (set = 0, binding = 1) uniform sampler2D tex_sampler;

layout (location = 0) in vec3 frag_position;
layout (location = 1) in vec3 frag_color;
layout (location = 2) in vec2 frag_tex_coord;
layout (location = 3) in vec4 frag_fog_color;
layout (location = 4) in float frag_fog_near;
layout (location = 5) in float frag_fog_far;
layout (location = 6) in float frag_fog_depth;
layout (location = 7) in float frag_fog_influence;

layout (location = 0) out vec4 out_color;

void main() {
    vec4 color = texture(tex_sampler, frag_tex_coord) * vec4(frag_color, 1.0);
    if (color.a < 0.001) {
        discard;
    }

    float fog_distance = length(frag_position);
    float fog_amount = smoothstep(frag_fog_near, frag_fog_far, fog_distance) * frag_fog_influence;

    out_color = mix(color, frag_fog_color, fog_amount);
}