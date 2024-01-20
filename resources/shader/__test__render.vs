
#extension GL_EXT_shader_explicit_arithmetic_types : enable

#extension GL_ARB_separate_shader_objects : enable

vec2 vertex_position[3] = vec2[](
	vec2 ( -0.5, 0.5 ),
	vec2 ( 0.5, 0.5 ),
	vec2 ( 0.0, -0.5 )
);

vec3 vertex_color[3] = vec3[](
	vec3 ( 1.0, 0.0, 0.0 ),
	vec3 ( 0.0, 1.0, 0.0 ),
	vec3 ( 0.0, 0.0, 1.0 )
);

layout (location = 0) out vec3 out_color;

void main() {
	gl_Position = vec4(vertex_position[gl_VertexIndex], 0.1, 1.0);
	out_color = vertex_color[gl_VertexIndex];
}