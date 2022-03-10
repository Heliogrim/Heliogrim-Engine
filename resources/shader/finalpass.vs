#version 450 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

vec2 triangle_positions[6] = vec2[](
    vec2(1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0)
);

vec3 triangle_colors[6] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 1.0, 0.0),
    vec3(1.0, 0.0, 0.0)
);

vec2 triangle_uv[6] = vec2[](
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0)
);

layout (location = 0) out vec2 out_uv;
layout (location = 1) out vec3 out_color;

void main() {
	gl_Position = vec4(triangle_positions[gl_VertexIndex], 0.0, 1.0);
    out_uv = triangle_uv[gl_VertexIndex];
    out_color = triangle_colors[gl_VertexIndex];

}