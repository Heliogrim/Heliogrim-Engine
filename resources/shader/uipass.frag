#version 450 core

/*
*/
layout (set = 0, binding = 1) uniform sampler2D mapped_texture;
/*
*/

layout (location = 0) in vec4 in_color;
layout (location = 1) in vec3 in_uvm;

layout (location = 0) out vec4 out_color;

void main() {

	vec4 sampled = texture(mapped_texture, vec2(in_uvm.xy)).rgba;
	//color = pow(color, vec3(1.0 / 2.2));
	out_color = in_color * sampled.rgba;
	/*
	out_color = vec4(
		in_color.rgb,
		1.0
	);
	*/
}