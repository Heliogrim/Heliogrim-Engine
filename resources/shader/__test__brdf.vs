
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 0) uniform CameraObj {
	mat4 vp;
	mat4 proj;
	mat4 view;
} co;

layout (set = 1, binding = 0) buffer ModelObj {
	mat4 model[];
} mo;

layout (location = 0) in vec3 inLocalPosition;
//layout (location = 1) in vec3 in_color;
layout (location = 2) in vec3 inLocalUv;
layout (location = 3) in vec3 inLocalNormal;

layout (location = 0) out vec4 outWorldPosition;
layout (location = 1) out vec4 outWorldNormal;
layout (location = 2) out vec3 outLocalUv;

void main() {
	/**
     * Attention: Need to guarantee invariant from gl_Position calculation
     *  If not guaranteed, z-fighting will occur
     *
     * @see https://stackoverflow.com/a/46920273
     */
	vec4 pos = mo.model[gl_InstanceIndex] * vec4(inLocalPosition, 1.0);
	gl_Position = co.vp * pos;

	outWorldPosition = pos;
	outLocalUv = inLocalUv;

	/**
	 *
	 */
	outWorldNormal = vec4(mat3(mo.model[gl_InstanceIndex]) * normalize(inLocalNormal), 0.0);
}