#version 450 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

/*
layout (set = 0, binding = 1) uniform FinalPassUbo {
    vec3 pos;
} ubo;

layout (set = 0, binding = 2) uniform sampler2D mapped_albedo;
layout (set = 0, binding = 3) uniform sampler2D mapped_normal;
layout (set = 0, binding = 4) uniform sampler2D mapped_position;
layout (set = 0, binding = 5) uniform sampler2D mapped_mrs;
layout (set = 0, binding = 6) uniform sampler2D mapped_depth;
*/

layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec4 out_color;

void main() {
	/**
	 * Get data from G-Buffer / Mrt-Buffer
	 */
	/*
	vec4 albedo = texture(mapped_albedo, in_uv);
	vec3 normal = normalize(texture(mapped_normal, in_uv).xyz);
	vec3 position = texture(mapped_position, in_uv).xzy;
	vec3 mrs = texture(mapped_mrs, in_uv).rgb;

	out_color = vec4(albedo.rgb, 1.0);
	*/
	out_color = vec4(in_color, 1.0);
}