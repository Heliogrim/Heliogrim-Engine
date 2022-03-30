#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(early_fragment_tests) in;

/*
*/
layout (set = 1, binding = 3) uniform sampler2D mapped_albedo;
layout (set = 1, binding = 4) uniform sampler2D mapped_normal;
layout (set = 1, binding = 5) uniform sampler2D mapped_roughness;
layout (set = 1, binding = 6) uniform sampler2D mapped_specular;
/*
*/

layout (location = 0) in vec4 fragPosition;
layout (location = 1) in vec4 fragNormal;
layout (location = 2) in vec3 fragUvm;

layout (location = 0) out vec4 out_albedo;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec4 out_position;
layout (location = 3) out vec4 out_mrs;

void main() {
	out_albedo = texture(mapped_albedo, fragUvm.st);
	out_normal = fragNormal.xyz;
	out_position = fragPosition;
	out_mrs = vec4(
		0.0,
		texture(mapped_roughness, fragUvm.st).r,
		texture(mapped_specular, fragUvm.st).r,
		1.0
	);
}