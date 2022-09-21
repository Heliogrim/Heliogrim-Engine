#version 450 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable

#define PI 3.1415926535897932384626433832795
#define EXPOSURE 4.5f

struct light {
	vec3 color; // Intensity Light-Color ( lightColor * intensity )
	vec3 position;
	vec3 direction;
	float limit; // Distance limit for light
};

/*
const light lights[4] = light[] (
	light(
		vec3(15.0),
		vec3(0.0, 0.0, 0.0),
		vec3(2.0, 5.0, 2.0),
		-1.0
	),
	light (
		vec3(78.3, 3.1, 2.6),
        vec3(6.7, 5.0, 6.7),
		vec3(0.0),
		8.0
	),
	light(
		vec3(19.3, 17.1, 83.2),
        vec3(-10.0, 4.8, 5.0),
		vec3(0.0),
		8.0
	),
	light(
		vec3(21.0, 63.0, 23.0),
        vec3(5.0, 4.3, -10.0),
		vec3(0.0),
		8.0
	)
);
*/
const light lights[4] = light[] (
	light(
		vec3(150.0),
		vec3(-10.0, 5.0, -10.0),
		vec3(0.0),
		84.0
	),
	light (
		vec3(150.0),
        vec3(-10.0, 5.0, 10.0),
		vec3(0.0),
		84.0
	),
	light(
		vec3(150.0),
        vec3(10.0, 5.0, -10.0),
		vec3(0.0),
		84.0
	),
	light(
		vec3(150.0),
        vec3(10.0, 5.0, -10.0),
		vec3(0.0),
		84.0
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

layout (set = 0, binding = 7) uniform sampler2D mapped_brdfLut;
layout (set = 0, binding = 8) uniform samplerCube mapped_prefilter;
layout (set = 0, binding = 9) uniform samplerCube mapped_irradiance;
/*
*/

layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec4 out_color;

float saturate(float t) {
	return clamp(t, 0.0, 1.0);
}

#define FP16_MAX float16_t(65504.0)

float16_t saturateFp16(float16_t t) {
	return min(t, FP16_MAX);
}

// Lights

#define INV_RADIUS (1.0f / l.limit)
#define INNER_ANGLE /*l.position.y*/0.785398
#define OUTER_ANGLE /*l.position.z*/1.0472

/**
 * .
 *
 * @author Julius
 * @date 26.12.2020
 *
 * @param dist Distance between light and position
 * @param imd Inverse Max Distance between ligth and position
 */
float spotlight_falloff(float dist, float imd) {
	float d2 = dist * dist;
	float fac = d2 * imd * imd;
	float sfac = max(1.0 - fac * fac, 0.0);
	return (sfac * sfac) / max(d2, 1e-4);
}

/**
 * .
 *
 * @author Julius
 * @date 26.12.2020
 *
 * @param ltp Light To Position
 * @param ld Light Direction
 * @param inner Inner-Angle of Spot in radians
 * @param outer Outer-Angle of Spot in radians
 */
float spotlight_angle(vec3 ltp, vec3 ld, float inner, float outer) {
	
	float co = cos(outer);
	float scale = 1.0 / max(cos(inner) - co, 1e-4);
	float off = -co * scale;

    // float ldl = dot(normalize(ld), ltp);
	float ldl = dot(normalize(ld), ltp);
	float fac = saturate(ldl * scale + off);
	return fac * fac;
}

/**
 * .
 *
 * @author Julius
 * @date 26.12.2020
 *
 * @param l Light
 * @param position Position
 */
vec3 light_spotlight(light l, vec3 position) {
	vec3 diff = l.position - position;
	vec3 ltp = normalize(diff); // Normalize Difference = Light To Position = Direction

	float attenuation = 1.0f *
		spotlight_falloff(length(diff), INV_RADIUS) *
		spotlight_angle(ltp, l.direction, INNER_ANGLE, OUTER_ANGLE);
	return l.color * attenuation/* * ndl Which is already in BRDF */;
}

#undef INV_RADIUS
#undef INNER_ANGLE
#undef OUTER_ANGLE

/**
 * .
 *
 * @author Julius
 * @date 27.12.2020
 *
 * @param l Light
 * @param position Position
 */
vec3 light_pointlight(light l, vec3 position) {
    return l.color * spotlight_falloff(length(l.position - position), 1.0f / l.limit);
}

/**
 * https://google.github.io/filament/Filament.md#lighting/imagebasedlights
 * 
 * [Light]   | 10am       | 12pm       | 05:30pm
 *
 * Sky + Sun | 120.000 lx | 130.000 lx | 90.000 lx
 * Sky       |  20.000 lx |  25.000 lx |  9.000 lx
 * Sun       | 100.000 lx | 105.000 lx | 81.000 lx
 *
 * Table 12: Illuminance values in lx (a full moon has an illuminance of 1 lx)
 */

/**
 * .
 *
 * @author Julius
 * @date 26.12.2020
 *
 * @param l Light
 * @param normal Normal at Position
 */
vec3 light_directional(light l, vec3 normal) {
	// float ndl = clamp(dot(normal, normalize(l.direction)), 0.0, 1.0);
	return l.color/* * ndl Which is already in BRDF */;
}

// !Lights

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

float16_t ndf_ggx_fp16(float16_t ndh, const f16vec3 n, const f16vec3 h, float16_t rough) {
	
	f16vec3 nxh = cross(n, h);
	float16_t a = ndh * rough;
	float16_t k = rough / (dot(nxh, nxh) + a * a);
	float16_t d = k * k * (float16_t(1.0) / float16_t(PI));

	return saturateFp16(d);
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
	
	float a2 = rough * rough;

	float ggxv = ndl * sqrt(ndv * ndv * (1.0 - a2) + a2);
	float ggxl = ndv * sqrt(ndl * ndl * (1.0 - a2) + a2);

	return 0.5 / (ggxv + ggxl);
}

/**
 * Fresnel Function - Schlick
 *
 * @author Julius
 * @date 27.12.2020
 */
vec3 fresnel_sch(float ct, vec3 F0) {
	// return F0 + (1.0 - F0) * pow(1.0 - ct, 5.0);
	float f = pow(1.0 - ct, 5.0);
	return f + F0 * (1.0 - f);
}

vec3 fresnel_sch(float ct, vec3 F0, float F90) {
	// Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
	return F0 + (F90 - F0) * pow(1.0 - ct, 5);
}

float fresnel_sch(float ct, float F0, float F90) {
	// Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
	return F0 + (F90 - F0) * pow(1.0 - ct, 5);
}

vec3 fresnel_sch_rough(float theta, vec3 F0, float rough) {
	return F0 + (max(vec3(1.0 - rough), F0) - F0) * pow(1.0 - theta, 5.0);
}

/**
 *
 */
float disney_diff(float ndvd, float ndl, float ldh, float rough) {

	// Burley 2012, "Physically-Based Shading at Disney"

	/*
	float F90 = 0.5 + 2.0 * rough * ldh * ldh;
	float ls = fresnel_sch(ndl, 1.0, F90);
	float vs = fresnel_sch(ndvd, 1.0, F90);
	return ls * vs * (1.0 / PI);
	*/

	/**
	 * Frostbite modified 'Disney-Diffuse Approx Renormalization'
	 *
	 * @see https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf#page=11&amp;zoom=auto,-265,590
	 */
	float bias = mix(0.0, 0.5, rough);
	float fac = mix(1.0, 1.0 / 1.51, rough);

	float Fd90 = bias + 2.0 * ldh * ldh * rough;
	const float F0 = 1.0;

	float lightScatter = fresnel_sch(ndl, F0, Fd90);
	float viewScatter = fresnel_sch(ndvd, F0, Fd90);

	return lightScatter * viewScatter * fac;
}

/**
 *
 */
vec3 prefilteredReflection(vec3 R, float rough) {
	
	const float MAX_REFLECTION_LOD = 9.0;
	float lod = rough * MAX_REFLECTION_LOD;
	float lodF = floor(lod);
	float lodC = ceil(lod);

	vec3 lowerSample = textureLod(mapped_prefilter, R, lodF).rgb;
	vec3 upperSample = textureLod(mapped_prefilter, R, lodC).rgb;

	return mix(lowerSample, upperSample, lod - lodF);
}

void main() {
	/**
	 * Get data from G-Buffer / Mrt-Buffer
	 */
	vec3 albedo = pow(texture(mapped_albedo, in_uv).rgb, vec3(2.2));
	vec3 normal = normalize(texture(mapped_normal, in_uv).xyz);
	vec4 position = texture(mapped_position, in_uv);
	vec3 mrs = texture(mapped_mrs, in_uv).rgb;
	vec3 depth = texture(mapped_depth, in_uv).rrr;

	//
	vec3 color = vec3(0.0);

	/**
	 *
	 */
	vec3 vd = normalize(ubo.pos - position.xyz);
	float ndvd = saturate(dot(normal, vd));

	vec3 reflection = reflect(-vd, normal);

	/**
	 * PBR Shading
	 */

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, mrs.r);

	float F90 = saturate(F0.r * 50.0);

	// reflectance equation
    vec3 Lo = vec3(0.0);

	for (int i = 0; i < 4; ++i) {
		// Current Light
		light L = lights[i];

		// calculate per-light radiance
        vec3 ltp = normalize(L.position - position.xyz); // Light to Position
        vec3 h = normalize(vd + ltp);

		vec3 radiance;
        float ndl;

		if (L.limit > 0) {

			if ((L.direction.x + L.direction.y + L.direction.z) < 0.0001) {
				
				// Point Light
				radiance = light_pointlight(L, position.xyz);

			} else {

				// Spotlight Light
				radiance = light_spotlight(L, position.xyz);
			}

			ndl = saturate(dot(normal, ltp));
			
		} else {
			// Directional Light
			radiance = light_directional(L, normal);
			ndl = saturate(dot(normal, normalize(L.direction)));
		}

		// precalculate values
        float ndh = saturate(dot(normal, h));
		float ldh = saturate(dot(ltp, h));
        float hdvd = max(dot(h, vd), 0.0);

		if (ndl > 0.0) {
			/**
			 * Specular BRDF
			 */
			float D = ndf_ggx(ndh, mrs.g); // Normal Distribution
			float G = geo_sch_smith(ndvd, ndl, mrs.g); // Geometric Shadowing
			vec3 F = fresnel_sch(ldh, F0, F90); // Fresnel Factor

			vec3 Fr = (D * G * F) / PI;

			/**
			 * Diffuse BRDF
			 */
			float Fd = disney_diff(ndvd, ndl, ldh, mrs.g) / PI;

			vec3 kD = 1.0 - F;
			vec3 dc = albedo * (1.0 - mrs.r);

			// add to outgoing radiance Lo
			Lo += (dc * kD * Fd + Fr) * (radiance * ndl);
		}
	}

	// Ambient Lighting
	vec3 kS = fresnel_sch_rough(ndvd, F0, mrs.g);
	vec3 kD = (1.0 - kS) * (1.0 - mrs.r);

	vec2 brdf = texture(mapped_brdfLut, vec2(dot(normal, vd), mrs.g)).rg;
	vec3 preflec = prefilteredReflection(reflection, mrs.g);
	vec3 irradiance = texture(mapped_irradiance, normal).rgb;

	// Ambient :: Irradiance Diffuse
	vec3 irrDiff = irradiance * albedo.rgb;

	// Ambient :: Specular Reflect
	vec3 rspec = preflec * (kS * brdf.x + brdf.y);


	vec3 ambient = (kD * irrDiff + rspec) * mrs.b;
	// vec3 ambient = irradiance * albedo.rgb /* * ao.r */ * 0.8;
	// vec3 ambient = (kD * irrDiff) * mrs.b;

	// Composite
	color = ambient + Lo;

	// Bypass Skybox Color
	if (depth.r >= 1.0) {
		color = texture(mapped_albedo, in_uv).rgb;
	}

	// HDR Tonemapping
	color = tonemap(color * EXPOSURE);
    color = color * ( 1.0 / tonemap(vec3(11.2)) );

	// Gamma Correction
	color = pow(color, vec3(1.0 / 2.2));

	out_color = vec4(color, 1.0);
}