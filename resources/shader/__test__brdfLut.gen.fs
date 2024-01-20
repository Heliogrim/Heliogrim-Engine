
layout (location = 0) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

layout (constant_id = 0) const uint SAMPLE_COUNT = 1024u;

#define PI 3.1415926535897932384626433832795

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

vec2 integrBrdf(float ndv, float rough) {

	// For pre-computed BRDF we assume that normal always points straight
	const vec3 N = vec3(0.0, 0.0, 1.0);
	const vec3 V = vec3(sqrt(1.0 - ndv * ndv), 0.0, ndv);

	vec2 Lut = vec2(0.0);
	for (uint i = 0u; i < SAMPLE_COUNT; ++i) {

		vec2 Xi = hammersley(i, SAMPLE_COUNT);
		vec3 h = importanceSample(Xi, N, rough);
		vec3 L = 2.0 * dot(V, h) * h - V;

		float ndl = max(dot(N, L), 0.0);
		float ndv = max(dot(N, V), 0.0);
		float vdh = max(dot(V, h), 0.0);
		float ndh = max(dot(h, N), 0.0);

		if (ndl > 0.0) {

			float G = geo_sch_smith(ndl, ndv, rough);
			float Gv = (G * vdh) / (ndh * ndv);
			float F = fresnel_sch(vdh, 0.0, 1.0);
			
			Lut += vec2((1.0 - F) * Gv, F * Gv);
		}
	}

	return Lut / float(SAMPLE_COUNT);
}

void main() {
	out_color = vec4(integrBrdf(in_uv.s, in_uv.t), 0.0, 1.0);
}