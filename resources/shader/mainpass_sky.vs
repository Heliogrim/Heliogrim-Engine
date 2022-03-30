#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

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

layout (set = 0, binding = 1) uniform SkyMainPassUbo {
	mat4 mvp;
} ubo;

//layout (location = 0) in vec3 in_position;
//layout (location = 1) in vec3 in_color;
//layout (location = 2) in vec3 in_uvm;
//layout (location = 3) in vec3 in_normal;

layout (location = 0) out vec3 fragUvw;

void main() {
	fragUvw = cube_vertices[gl_VertexIndex];
	fragUvw.xy * -1.0;
	gl_Position = ubo.mvp * vec4(cube_vertices[gl_VertexIndex], 1.0);
	// gl_Position.z = gl_Position.w;
}