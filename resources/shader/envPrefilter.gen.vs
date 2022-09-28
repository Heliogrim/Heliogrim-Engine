#version 450 core

#extension GL_EXT_multiview : enable

// Warning: Copied from 'mainpass_sky.vs'
vec3 cube_vertices[36] = vec3[](
	// Front
	vec3(-1.0, -1.0, -1.0),
	vec3(-1.0, 1.0, -1.0),
	vec3(1.0, 1.0, -1.0),

	vec3(1.0, 1.0, -1.0),
	vec3(1.0, -1.0, -1.0),
	vec3(-1.0, -1.0, -1.0),

	// Right
	vec3(1.0, -1.0, -1.0),
	vec3(1.0, 1.0, -1.0),
	vec3(1.0, 1.0, 1.0),

	vec3(1.0, 1.0, 1.0),
	vec3(1.0, -1.0, 1.0),
	vec3(1.0, -1.0, -1.0),

	// Back
	vec3(1.0, -1.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(-1.0, 1.0, 1.0),

	vec3(-1.0, 1.0, 1.0),
	vec3(-1.0, -1.0, 1.0),
	vec3(1.0, -1.0, 1.0),

	// Left
	vec3(-1.0, -1.0, 1.0),
	vec3(-1.0, 1.0, 1.0),
	vec3(-1.0, 1.0, -1.0),

	vec3(-1.0, 1.0, -1.0),
	vec3(-1.0, -1.0, -1.0),
	vec3(-1.0, -1.0, 1.0),

	// Top
	vec3(-1.0, 1.0, -1.0),
	vec3(-1.0, 1.0, 1.0),
	vec3(1.0, 1.0, 1.0),

	vec3(1.0, 1.0, 1.0),
	vec3(1.0, 1.0, -1.0),
	vec3(-1.0, 1.0, -1.0),

	// Bottom
	vec3(1.0, -1.0, -1.0),
	vec3(1.0, -1.0, 1.0),
	vec3(-1.0, -1.0, 1.0),

	vec3(-1.0, -1.0, 1.0),
	vec3(-1.0, -1.0, -1.0),
	vec3(1.0, -1.0, -1.0)
);

layout (set = 0, binding = 1) uniform EnvPrefilterUbo {
	mat4 mvp[6];
} ubo;

layout (location = 0) out vec3 fragUvw;

void main() {

	mat4 umvp = ubo.mvp[gl_ViewIndex];

	// Warning: Copied from 'mainpass_sky.vs'
	fragUvw = cube_vertices[gl_VertexIndex];
	fragUvw.xy * -1.0;
	gl_Position = umvp * vec4(cube_vertices[gl_VertexIndex], 1.0);
}