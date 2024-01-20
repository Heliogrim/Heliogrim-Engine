
vec2 triangle_positions[6] = vec2[](
    vec2(1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0)
);

vec2 triangle_uv[6] = vec2[](
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0)
);

layout (location = 0) out vec2 out_uv;

void main() {
	gl_Position = vec4(triangle_positions[gl_VertexIndex], 0.0, 1.0);
    out_uv = triangle_uv[gl_VertexIndex];
}