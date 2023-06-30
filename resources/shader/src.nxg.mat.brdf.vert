#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

/* === */

// Platform Source Stage

// Error: (set:0,bind1) and (set:1,bind:1) will conflict

layout uniform $bind/camera {
	// Warning: Currently not supported
    // mat4 $bind/camera/viewProj;
	mat4 viewProj;
};

layout buffer $bind/model {
	// Warning: Currently not supported
    // mat4 $bind/model/model;
	mat4 model;
};

// --

layout in vec3 $in/vertex/position;
layout in vec3 $in/vertex/uvm;
layout in vec3 $in/vertex/normal;

// --

layout out vec4 $out/position;
layout out vec4 $out/normal;
layout out vec3 $out/uvm;

/* === */

void main() {
	/**
     * Attention: Need to guarantee invariant from gl_Position calculation
     *  If not guaranteed, z-fighting will occur
     *
     * @see https://stackoverflow.com/a/46920273
     */
	vec4 pos = $bind/model.model * vec4($in/vertex/position, 1.0);
	gl_Position = $bind/camera.viewProj * pos;

	$out/position = pos;
	$out/uvm = $in/vertex/uvm;

	/**
	 *
	 */
	$out/normal = vec4(mat3($bind/model.model) * normalize($in/vertex/normal), 0.0);
}