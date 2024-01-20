//#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

vec2 triangle_positions[6] = vec2[](
    vec2(1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, -1.0),
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

vec2 triangle_uv[6] = vec2[](
    vec2(1.0, 1.0),
    vec2(1.0, 0.0),
    vec2(0.0, 0.0),
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
);

layout (location = 0) out vec2 outLocalUv;

void main() {
	gl_Position = vec4(triangle_positions[gl_VertexIndex], 0.0, 1.0);
    outLocalUv = triangle_uv[gl_VertexIndex];
}