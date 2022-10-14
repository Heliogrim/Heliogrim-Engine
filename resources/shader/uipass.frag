#version 450 core

/*
*/
layout (set = 0, binding = 1) uniform sampler2D mapped_texture;
/*
*/

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec3 in_uvm;

layout (location = 0) out vec4 out_color;

void main() {

	vec4 sampled = texture(mapped_texture, vec2(in_uvm.xy)).rgba;
	vec3 color = in_color * sampled.rgb;
	//color = pow(color, vec3(1.0 / 2.2));
	out_color = vec4(
		color,
		sampled.a
	);
	/*
	out_color = vec4(
		in_color.rgb,
		1.0
	);
	*/
}