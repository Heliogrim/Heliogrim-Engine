
#extension GL_EXT_shader_explicit_arithmetic_types : enable

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 in_color;

layout (location = 0) out vec4 out_color;

void main() {
	out_color = vec4(in_color, 1.0);
}