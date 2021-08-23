#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 2) in vec3 inTexCoord;

layout (location = 0) out vec2 fragUV;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() {
	fragUV = inTexCoord.xy;
	// gl_Position = vec4(inPosition.x * 2.0 - 1.0, inPosition.y * 2.0 - 1.0, inPosition.z, 1.0);

	// Render mrt image on the lower end of the depth buffer
	gl_Position = vec4(inPosition.x * 2.0 - 1.0, inPosition.y * 2.0 - 1.0, 1.0, 1.0);
}