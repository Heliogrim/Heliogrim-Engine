#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform samplerCube tex;

layout (location = 0) in vec3 fragUVW;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outDiffuse;
layout (location = 4) out vec4 outSpecular;

void main()
{
	outAlbedo = texture(tex, fragUVW);
}