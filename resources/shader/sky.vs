#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inUvm;
layout (location = 3) in vec3 inNormal;

layout (set = 0, binding = 1) uniform CameraObj {
	mat4 mvp;
	mat4 proj;
	mat4 view;
} co;

layout (location = 0) out vec3 fragUvw;

void main() {

	fragUvw = inPosition * 2.0;
	gl_Position = co.proj * mat4(mat3(co.view)) * vec4(inPosition.xyz * 2.0, 1.0);
	gl_Position.z = gl_Position.w;
}