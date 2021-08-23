#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inUvm;
layout (location = 3) in vec3 inNormal;

layout (set = 0, binding = 1) uniform CameraObj {
	mat4 mvp;
} co;

//layout (set = 0, binding = 3) uniform sampler2D heightmap;

layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec2 fragUV;

void main() {

	gl_Position = co.mvp * vec4(inPosition * 128.0, 1.0);

	fragPosition = inPosition * 128.0;
	fragNormal = inNormal;
	fragUV = inUvm.xy;
}