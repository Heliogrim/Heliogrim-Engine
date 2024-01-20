
#extension GL_EXT_shader_explicit_arithmetic_types : enable

#extension GL_ARB_separate_shader_objects : enable

vec2 vertex_position[3] = vec2[](
	vec2 ( -0.5, 0.5 ),
	vec2 ( 0.5, 0.5 ),
	vec2 ( 0.0, -0.5 )
);

void main() {
	gl_Position = vec4(vertex_position[gl_VertexIndex], 0.2, 1.0);
}