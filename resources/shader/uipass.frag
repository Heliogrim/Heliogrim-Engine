#version 450 core

/*
*/
layout (set = 0, binding = 1) uniform sampler2DArray mapped_texture;
/*
*/

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec3 in_uvm;

layout (location = 0) out vec4 out_color;

void main() {

	out_color = vec4(
		in_color * texture(
			mapped_texture,
			vec3(
				in_uvm.xy,
				floor(in_uvm.z)
			)
		).rgb,
		1.0
	);

}