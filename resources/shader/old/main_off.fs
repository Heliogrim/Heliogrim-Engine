#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2DArray texArray;

layout (location = 0) in vec3 fragUVW;

//layout (location = 0) out vec4 outColor;
layout (location = 0) out float outDepth;

void main()
{
	if (texture(texArray, fragUVW).a < 0.5)
		discard;
	outDepth = gl_FragCoord.z;
}