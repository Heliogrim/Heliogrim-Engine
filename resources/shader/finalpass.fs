#version 450 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define PI 3.1415926535897932384626433832795
#define EXPOSURE 4.5f

struct light {
	vec3 color; // Intensity and Color ( color * intensity )
	vec3 position;
};

const light lights[1] = light[] (
	light (
		vec3(1.0),
		vec3(0.0, 5.0, 0.0)
	)
);

/*
*/
layout (set = 0, binding = 1) uniform FinalPassUbo {
    vec3 pos;
} ubo;

layout (set = 0, binding = 2) uniform sampler2D mapped_albedo;
layout (set = 0, binding = 3) uniform sampler2D mapped_normal;
layout (set = 0, binding = 4) uniform sampler2D mapped_position;
layout (set = 0, binding = 5) uniform sampler2D mapped_mrs;
layout (set = 0, binding = 6) uniform sampler2D mapped_depth;
/*
*/

layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec4 out_color;

// http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 tonemap(vec3 src) {
    const float A = 0.15f;
    const float B = 0.50f;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.02f;
    const float F = 0.30f;

    return ( ( src * ( A * src + C * B ) + D * E ) / ( src * ( A * src + B ) + D * F ) ) - E / F;
}

void main() {
	/**
	 * Get data from G-Buffer / Mrt-Buffer
	 */
	vec4 albedo = texture(mapped_albedo, in_uv);
	vec3 normal = normalize(texture(mapped_normal, in_uv).xyz);
	vec4 position = texture(mapped_position, in_uv);
	vec3 mrs = texture(mapped_mrs, in_uv).rgb;
	vec3 depth = texture(mapped_depth, in_uv).rrr;

	//
	vec3 color = vec3(0.0);

	/**
	 *
	 */
	vec3 vd = normalize(position.xyz - ubo.pos);

	for (int i = 0; i < 1; ++i) {
		// Current Light
		light L = lights[i];

		vec3 ld = normalize(L.position - position.xyz);

		float dp = max(dot(normal, ld), 0.0);
		vec3 diffuse = dp * L.color;

		vec3 R = reflect(-ld, normal);
		float sp = pow(max(dot(vd, R), 0.0), 32);
		vec3 specular = mrs.b * sp * L.color;

		//
		color = color + diffuse + specular;
	}

	// Ambient Lighting
	vec3 irradiance = vec3(0.03f);
	vec3 ambient = irradiance * albedo.rgb;

	// Composite
	color = ambient + color * albedo.rgb;

	// HDR Tonemapping
	color = tonemap(color * EXPOSURE);
	color = color * ( 1.0f / tonemap(vec3(11.2f)) );

	// Gamma Correction
	color = pow(color, vec3(1.0f / 2.2f));

	out_color = vec4(color, 1.0);
	out_color = vec4(albedo.rgb, 1.0);
}