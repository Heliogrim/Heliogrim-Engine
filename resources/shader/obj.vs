#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

vec2 triangle_positions[6] = vec2[](
    vec2(1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0)
);

/*
vec3 triangle_positions[6] = vec3[](
    vec3(1.0, 0.5, 0.1),
    vec3(1.0, -0.5, 0.1),
    vec3(-1.0, -0.5, 0.1),
    vec3(-1.0, -0.5, 0.1),
    vec3(-1.0, 0.5, 0.1),
    vec3(1.0, 0.5, 0.1)
);
*/

vec2 triangle_uv[6] = vec2[](
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0)
);

/*
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inUvm;
layout (location = 3) in vec3 inNormal;

layout (location = 4) in vec3 pinPosition;
layout (location = 5) in vec3 pinRotation;
layout (location = 6) in vec3 pinScale;
layout (location = 7) in uint pinMaterial;
*/

layout (set = 0, binding = 1) uniform CameraObj {
	mat4 mvp;
} co;

layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec3 fragUvm;

void main() {
	
	/*
    gl_Position = co.mvp * vec4(inPosition, 1.0);

	fragPosition = inPosition;
	fragNormal = inNormal;
	fragUvm = vec3(inUvm.xy, pinMaterial);
    */

    // Debugging
    vec4 p = vec4(triangle_positions[gl_VertexIndex] * 0.75, 0.0, 1.0);

	gl_Position = p;
    fragPosition = p.xyz;
    fragUvm = vec3(triangle_uv[gl_VertexIndex], 0.0);
    fragNormal = vec3(0.0);
}