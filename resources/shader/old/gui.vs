#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inTexCoord;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	// gl_Position = vec4(inPosition.x * 2.0 - 1.0, inPosition.y * 2.0 - 1.0, inPosition.z, 1.0);
	// gl_Position = vec4(inPosition.x * 2.0 - 1.0, inPosition.y * 2.0 - 1.0, 0.9, 1.0);
	gl_Position = vec4(inPosition.x * 2.0 - 1.0, inPosition.y * 2.0 - 1.0, -1.0, 1.0);
}