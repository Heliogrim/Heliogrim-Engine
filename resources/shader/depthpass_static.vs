#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 1) uniform DepthPassUbo {
	mat4 viewProj;
} ubo;

layout (set = 0, binding = 2) buffer DepthPassModel {
	mat4 model[];
} mubo;

layout (location = 0) in vec3 in_position;

void main() {
	/**
     * Attention: Need to guarantee invariant from gl_Position calculation
     *  If not guaranteed, z-fighting will occur
     *
     * @see https://stackoverflow.com/a/46920273
     */
	vec4 pos = mubo.model[gl_InstanceIndex] * vec4(in_position, 1.0);
    gl_Position = ubo.viewProj * pos;
}