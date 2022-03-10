#version 450

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

layout (set = 0, binding = 1) uniform DepthPassUbo {
	mat4 mvp;
} ubo;

layout (location = 0) in vec3 in_position;

void main() {
	// gl_Position = ubo.mvp * vec4(in_position, 1.0);
    gl_Position = vec4(triangle_positions[gl_VertexIndex], 0.2, 1.0);
}