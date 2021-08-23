#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UniformBufferObj
{
	mat4 mvp;
} ubo;

layout (location = 0) out vec3 fragUVW;

void main()
{
	fragUVW = inPos;
	fragUVW.x *= -1.0;
	fragUVW.y *= -1.0;
	
	gl_Position = ubo.mvp * vec4(inPos.xyz, 1.0);
}
