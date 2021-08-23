#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 2) in vec3 inUvm;

void main() {
	
	gl_Position = vec4(inPosition.x * 2.0 - 1.0, inPosition.y * 2.0 - 1.0, -1.0, 1.0);
}