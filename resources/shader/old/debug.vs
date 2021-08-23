#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// Vertex Attributes
layout (location = 0) in vec3 inPosition;
layout (location = 3) in vec3 inNormal;

// Instance Attributes
layout (location = 4) in vec3 instPosition;
layout (location = 5) in vec3 instRotation;
layout (location = 6) in float instScale;

layout (location = 0) out vec3 outNormal;

out gl_PerVertex
{
	vec4 gl_Position;
};

vec4 getWorldPos()
{
	mat3 mx,my,mz;

	float s = sin(instRotation.x);
	float c = cos(instRotation.x);

	mx[0] = vec3(c, s, 0.0);
	mx[1] = vec3(-s, c, 0.0);
	mx[2] = vec3(0.0, 0.0, 1.0);

	s = sin(instRotation.y);
	c = cos(instRotation.y);

	my[0] = vec3(c, 0.0, s);
	my[1] = vec3(0.0, 1.0, 0.0);
	my[2] = vec3(-s, 0.0, c);

	s = sin(instRotation.z);
	c = cos(instRotation.z);

	mz[0] = vec3(1.0, 0.0, 0.0);
	mz[1] = vec3(0.0, c, s);
	mz[2] = vec3(0.0, -s, c);

	mat3 rot = mz * my * mx;
	vec4 pos = vec4(inPosition.xyz * rot, 1.0);
	return vec4((pos.xyz * instScale) + instPosition, 1.0);
}

void main()
{
	outNormal = inNormal;
	gl_Position = getWorldPos();
}
