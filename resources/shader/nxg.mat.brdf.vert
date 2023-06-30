#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

/* === */

// Platform Source Stage

// Error: (set:0,bind1) and (set:1,bind:1) will conflict

layout (set = 0, binding = 0) uniform __bind_camera_t {
	// Warning: Currently not supported
    // mat4 __bind_camera/viewProj;
	mat4 viewProj;
}__bind_camera;

layout (set = 1, binding = 0) buffer __bind_model_t {
	// Warning: Currently not supported
    // mat4 __bind_model/model;
	mat4 model;
}__bind_model;

// --

layout (location = 0) in vec3 __in_vertex_position;
layout (location = 1) in vec3 __in_vertex_color;
layout (location = 2) in vec3 __in_vertex_uvm;
layout (location = 3) in vec3 __in_vertex_normal;

// --

layout (location = 0) out vec4 __out_position;
layout (location = 1) out vec4 __out_normal;
layout (location = 2) out vec3 __out_uvm;

/* === */

void main() {
	/**
     * Attention: Need to guarantee invariant from gl_Position calculation
     *  If not guaranteed, z-fighting will occur
     *
     * @see https://stackoverflow.com/a/46920273
     */
	vec4 pos = __bind_model.model * vec4(__in_vertex_position, 1.0);
	gl_Position = __bind_camera.viewProj * pos;

	__out_position = pos;
	__out_uvm = __in_vertex_uvm;

	/**
	 *
	 */
	__out_normal = vec4(mat3(__bind_model.model) * normalize(__in_vertex_normal), 0.0);
}
