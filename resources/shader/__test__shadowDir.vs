
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#extension GL_EXT_multiview : enable

layout (std430, set = 0, binding = 0) buffer CameraObj {
	mat4 cascadeViewProj[4];
	vec4 cascadeDepth;
} co;

layout (set = 1, binding = 0) buffer ModelObj {
	mat4 model[];
} mo;

layout (push_constant) uniform PushConsts {
	uint cascade;
} push;

layout (location = 0) in vec3 inLocalPosition;

out gl_PerVertex {
    vec4 gl_Position;   
};

void main() {
	/**
     * Attention: Need to guarantee invariant from gl_Position calculation
     *  If not guaranteed, z-fighting will occur
     *
     * @see https://stackoverflow.com/a/46920273
     */
	vec4 pos = mo.model[gl_InstanceIndex] * vec4(inLocalPosition, 1.0);
	gl_Position = co.cascadeViewProj[push.cascade] * pos;
}