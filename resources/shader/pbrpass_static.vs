#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 1) uniform PbrPassUbo {
	mat4 viewProj;
} ubo;

layout (set = 0, binding = 2) buffer PbrPassModel {
	mat4 model[];
} mubo;

layout (location = 0) in vec3 in_position;
//layout (location = 1) in vec3 in_color;
layout (location = 2) in vec3 in_uvm;
layout (location = 3) in vec3 in_normal;

layout (location = 0) out vec4 fragPosition;
layout (location = 1) out vec4 fragNormal;
layout (location = 2) out vec3 fragUvm;

void main() {
	/**
     * Attention: Need to guarantee invariant from gl_Position calculation
     *  If not guaranteed, z-fighting will occur
     *
     * @see https://stackoverflow.com/a/46920273
     */
	vec4 pos = mubo.model[gl_InstanceIndex] * vec4(in_position, 1.0);
	gl_Position = ubo.viewProj * pos;

	vec4 normal = mubo.model[gl_InstanceIndex] * vec4(in_normal, 0.0);

	fragPosition = pos;
	fragNormal = vec4(normalize(normal.xyz), 1.0);
	fragUvm = in_uvm;
}