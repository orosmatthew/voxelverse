#version 460

layout (set = 0, binding = 0) uniform GlobalUniform {
    mat4 view;
    mat4 proj;
    vec4 fog_color;
    float fog_near;
    float fog_far;
} global_ubo;

layout (set = 1, binding = 0) uniform ObjectUnifom {
    mat4 model;
    float fog_influence;
} object_ubo;

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec2 in_tex_coord;

layout (location = 0) out vec3 frag_position;
layout (location = 1) out vec3 frag_color;
layout (location = 2) out vec2 frag_tex_coord;
layout (location = 3) out vec4 frag_fog_color;
layout (location = 4) out float frag_fog_near;
layout (location = 5) out float frag_fog_far;
layout (location = 6) out float frag_fog_depth;
layout (location = 7) out float frag_fog_influence;

void main() {
    gl_Position = global_ubo.proj * global_ubo.view * object_ubo.model * vec4(in_pos, 1.0);

    frag_position = (global_ubo.view * vec4(in_pos, 1.0)).xyz;
    frag_color = in_color;
    frag_tex_coord = in_tex_coord;
    frag_fog_color = global_ubo.fog_color;
    frag_fog_near = global_ubo.fog_near;
    frag_fog_far = global_ubo.fog_far;
    frag_fog_depth = - (global_ubo.view * vec4(in_pos, 1.0)).z;
    frag_fog_influence = object_ubo.fog_influence;
}