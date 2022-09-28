#version 450 core

/*
*/
layout (set = 1, binding = 2) uniform samplerCube mapped_env;
/*
*/
layout (push_constant) uniform PushConsts {
	layout (offset = 0) float roughness;
	layout (offset = 4) uint samples;
} consts;
/*
*/

layout (location = 0) in vec3 fragUvw;

layout (location = 0) out vec4 out_color;

#define PI 3.1415926535897932384626433832795

float saturate(float t) {
	return clamp(t, 0.0, 1.0);
}

// Based omn http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
float random(vec2 co) {
	float a = 12.9898;
	float b = 78.233;
	float c = 43758.5453;
	float dt= dot(co.xy ,vec2(a,b));
	float sn= mod(dt,3.14);
	return fract(sin(sn) * c);
}

/**
 * Normal Distribution Function - GGX
 *
 * @author Julius
 * @date 27.12.2020
 *
 * @see https://github.com/google/filament/blob/main/shaders/src/brdf.fs#L54
 */
float ndf_ggx(float ndh, float rough) {

	float a2 = rough * rough;
	float f = (ndh * a2 - ndh) * ndh + 1.0;
	return a2 / (PI * f * f);
}

/**
 * Geometric Shadowing Function - Schlick, Smith
 *
 * @author Julius
 * @date 27.12.2020
 *
 * @see https://github.com/google/filament/blob/main/shaders/src/brdf.fs
 */
float geo_sch_smith(float ndv, float ndl, float rough) {
	
	/*
	float a2 = rough * rough;

	float ggxv = ndl * sqrt(ndv * ndv * (1.0 - a2) + a2);
	float ggxl = ndv * sqrt(ndl * ndl * (1.0 - a2) + a2);

	float ggx = ggxv + ggxl;
	if (ggx > 0.0) {
		return 0.5 / ggx;
	}
	return 0.0;
	*/
	float k = (rough * rough) / 2.0;
	float GL = ndl / (ndl * (1.0 - k) + k);
	float GV = ndv / (ndv * (1.0 - k) + k);
	return GL * GV;
}

float fresnel_sch(float ct, const float F0, float F90) {
	// Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
	return F0 + (F90 - F0) * pow(1.0 - ct, 5);
}

/**
 * Importance Sampling
 *
 * @see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf
 */
vec3 importanceSample(vec2 Xi, vec3 normal, float rough) {
	
	//
	float a = rough * rough;

	float phi = 2.0 * PI * Xi.x + random(normal.xz) * 0.1;
	float ct = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float st = sqrt(1.0 - ct * ct);

	vec3 h = vec3(st * cos(phi), st * sin(phi), ct);

	// Tangent Space
	vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, normal));
	vec3 bitangent = normalize(cross(normal, tangent));

	// To world space
	return normalize(tangent * h.x + bitangent * h.y + normal * h.z);
}

vec2 hammersley(uint i, uint count) {
	
	/**
	 * Radical inverse
	 *
	 * @see http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
	 */
	uint bits = (i << 16u) | (i >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

	float ri = float(bits) * 2.3283064365386963e-10;
	return vec2(float(i)/float(count), ri);
}

vec3 prefilterEnv(vec3 R, float rough) {

	//
	vec3 N = R;
	vec3 V = R;

	vec3 color = vec3(0.0);
	float weight = 0.0;
	float dim = float(textureSize(mapped_env, 0).s);

	for (uint i = 0u; i < consts.samples; ++i) {

		vec2 Xi = hammersley(i, consts.samples);
		vec3 h = importanceSample(Xi, N, rough);
		vec3 L = 2.0 * dot(V, h) * h - V;

		float ndl = saturate(dot(N, L));

		// @see https://placeholderart.wordpress.com/2015/07/28/implementation-notes-runtime-environment-map-filtering-for-image-based-lighting/
		if (ndl > 0.0) {

			float ndh = saturate(dot(N, h));
			float vdh = saturate(dot(V, h));

			float pdf = ndf_ggx(ndh, rough) * ndh / (4.0 * vdh) + 0.0001; // Probability Distribution

			float omegaS = 1.0 / (float(consts.samples) * pdf); // Slid angle
			float omegaP = 4.0 * PI / (6.0 * dim * dim); // Solid angle

			float mip = rough == 0.0 ? 0.0 : max(0.5 * log2(omegaS / omegaP) + 1.0, 0.0f); // Biased Mip (+1.0) to improve quality
			color += textureLod(mapped_env, L, mip).rgb * ndl;
			weight += ndl;
		}
	}

	return (color / weight);
}

void main() {

	vec3 N = normalize(fragUvw);
	out_color = vec4(prefilterEnv(N, consts.roughness), 1.0);
}