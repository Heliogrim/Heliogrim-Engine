#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(early_fragment_tests) in;

layout (location = 0) in vec4 fragPosition;
layout (location = 1) in vec4 fragNormal;
layout (location = 2) in vec3 fragUvm;

layout (location = 0) out vec4 out_position;
layout (location = 1) out vec4 out_normal;
layout (location = 2) out vec4 out_meta;

void main() {
	out_position = fragPosition;
	out_normal = fragNormal;
	out_meta = vec4(fragUvm, 0.0);
}