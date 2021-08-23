#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//layout (location = 0) out vec4 outColor;
layout (location = 0) out float outDepth;

void main()
{
	outDepth = gl_FragCoord.z;
}