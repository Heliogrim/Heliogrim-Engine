#version 450

vec2 triangle_positions[6] = vec2[](
    vec2(1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0)
);

vec3 triangle_colors[6] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 1.0, 0.0),
    vec3(1.0, 0.0, 0.0)
);

vec2 triangle_uv[6] = vec2[](
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0)
);

//layout (location = 0) in vec3 inPosition;
//layout (location = 1) in vec3 inUvm;

layout (location = 0) out vec2 fragUv;
layout (location = 1) out vec3 fragColor;


void main() {
	
	/*
    gl_Position = vec4(inPosition.x * 2.0 - 1.0, inPosition.y * 2.0 - 1.0, 1.0, 1.0);

	fragUv = inUvm.xy;
    */

    // Debugging
	gl_Position = vec4(triangle_positions[gl_VertexIndex], 0.0, 1.0);
    fragUv = triangle_uv[gl_VertexIndex];
    fragColor = triangle_colors[gl_VertexIndex];
}