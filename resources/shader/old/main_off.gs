#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define SHADOW_MAP_CASCADE_COUNT 4

layout (triangles, invocations = SHADOW_MAP_CASCADE_COUNT) in;
layout (triangle_strip, max_vertices = 3) out;

layout (binding = 0) uniform UniformBufferObj
{
	mat4 mvp[SHADOW_MAP_CASCADE_COUNT];
} ubo;

layout (location = 0) in vec3 inFragUV[];
layout (location = 0) out vec3 outFragUV;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	outFragUV = inFragUV[0];
	for (int i = 0; i < gl_in.length(); i ++)
	{
		gl_Layer = gl_InvocationID;
		gl_Position = ubo.mvp[gl_InvocationID] * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}