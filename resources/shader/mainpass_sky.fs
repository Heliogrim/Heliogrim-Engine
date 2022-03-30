#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(early_fragment_tests) in;

/*
*/
layout (set = 1, binding = 2) uniform samplerCube skyCubeMap;
/*
*/

layout (location = 0) in vec3 fragUvw;

layout (location = 0) out vec4 out_albedo;

void main() {
	out_albedo = texture(skyCubeMap, fragUvw);
}