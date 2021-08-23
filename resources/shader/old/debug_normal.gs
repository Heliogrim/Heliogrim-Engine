#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

layout (binding = 0) uniform UniformBufferObject
{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 lightSpace;
	vec3 lightPos;
} ubo;

layout (location = 0) in vec3 inNormal[];

layout (location = 0) out vec3 outColor;

void main()
{
	mat4 mvp = ubo.projMatrix * ubo.viewMatrix * ubo.modelMatrix;
	float l = 0.04;
	
	for (int i = 0; i < gl_in.length(); i ++)
	{
		vec3 pos = gl_in[i].gl_Position.xyz;
		vec3 normal = inNormal[i].xyz;
		
		gl_Position = mvp * vec4(pos, 1.0);
		outColor = vec3(1.0, 0.0, 0.0);
		EmitVertex();
		
		gl_Position = mvp * vec4(pos + normal * l, 1.0);
		outColor = vec3(0.0, 0.0, 1.0);
		EmitVertex();
		
		EndPrimitive();
	}
}