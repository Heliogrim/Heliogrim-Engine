#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 fragUvw;

layout (set = 1, binding = 2) uniform samplerCube skybox;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outMetalness;
layout (location = 4) out vec4 outRoughness;
layout (location = 5) out vec4 outAo;

void main() {

	outPosition = vec4(0.0, 0.0, 99999999.9, 0.0);
	outNormal = vec4(0.0);
	outAlbedo = vec4(texture(skybox, fragUvw).rgb, 0.0);
	outMetalness = vec4(0.0);
	outRoughness = vec4(0.0);
	outAo = vec4(0.0);
}