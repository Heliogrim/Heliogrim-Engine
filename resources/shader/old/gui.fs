#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
//#extension GL_ARB_shader_image_load_store : enable

//layout (early_fragment_tests) in;

layout (location = 0) out vec4 outColor;

void main() {
	outColor = vec4(1.0, 0.0, 0.0, 0.1);
}