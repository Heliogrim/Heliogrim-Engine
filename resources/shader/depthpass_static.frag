#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : enable

layout(early_fragment_tests) in;

/*
*/
layout (set = 2, binding = 3) uniform usampler2D mapped_albedo;
/*
*/

layout (location = 0) in vec3 fragUvm;

layout (location = 0) out uint32_t out_color;

void main() {
	//out_color = texture(mapped_albedo, fragUvm.st).r;

	uint32_t val = 0;
	val |= 1 << 16;
	val |= texture(mapped_albedo, fragUvm.st).r;
	/*
	*/
	/*
	val |= textureLod(mapped_albedo, fragUvm.st, 14.0).r;
	val &= 0xFFFF0FFF;
	val |= 0x00003000;
	*/


	out_color = val;
}