
#extension GL_EXT_shader_explicit_arithmetic_types : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_sparse_texture2 : enable
#extension GL_ARB_sparse_texture_clamp : enable

#define PI 3.1415926535897932384626433832795
#define EXPOSURE 4.5f

/**
 * IEEE 754
 *
 * float-point binary 16-bit = 2^-10 ~ 9.77e-04 machine epislon
 * float-point binary 32-bit = 2^-23 ~ 1.19e-07 machine epislon
 */
#define epsilon_f16 0.0001
#define epsilon_f32 0.0000001

struct GlslLight {
	vec4 luminance;
    vec4 position;
    vec4 direction;
    float limit;
};

#define SHADOW_CASCADE_COUNT 4u
struct GlslDirShadow {
	mat4 cascadeViewProj[SHADOW_CASCADE_COUNT];
	f32vec4 cascadeDepth;
};

/*
*/
layout (set = 0, binding = 0) uniform CameraObj {
	mat4 mvp;
	mat4 proj;
	mat4 view;
} co;

layout (set = 2, binding = 0) uniform SceneLightInfoObj {
	uint32_t sceneLightCount;
} sli;

layout (std140, set = 3, binding = 0) buffer SceneLightsObj {
	GlslLight lights[];
} sl;

layout (set = 4, binding = 0) uniform texture2D brdfLut;
layout (set = 5, binding = 0) uniform sampler brdfLutSampler; 
layout (set = 6, binding = 0) uniform textureCube brdfPref;
layout (set = 7, binding = 0) uniform sampler brdfPrefSampler;
layout (set = 8, binding = 0) uniform textureCube brdfIrrad;
layout (set = 9, binding = 0) uniform sampler brdfIrradSampler;

layout (set = 10, binding = 0) uniform texture2D brdfAlbedo;
layout (set = 11, binding = 0) uniform sampler brdfAlbedoSampler;
layout (set = 12, binding = 0) uniform texture2D brdfNormal;
layout (set = 13, binding = 0) uniform sampler brdfNormalSampler;
layout (set = 14, binding = 0) uniform texture2D brdfArm;
layout (set = 15, binding = 0) uniform sampler brdfArmSampler;

layout (set = 16, binding = 0) buffer SceneShadowsObj {
	GlslDirShadow shadows[];
} sso;
layout (set = 17, binding = 0) uniform texture2DArray shadowDir;
layout (set = 18, binding = 0) uniform sampler shadowDirSampler;
/*
*/

layout (location = 0) in vec4 inWorldPosition;
layout (location = 1) in vec4 inWorldNormal;
layout (location = 2) in vec3 inLocalUv;

layout (location = 0) out vec4 outShadingColor;

float saturate(float t) {
	return clamp(t, 0.0, 1.0);
}

vec3 saturate(vec3 x) {
    return clamp(x, 0.0, 1.0);
}

#define FP16_MAX float16_t(65504.0)

float16_t saturateFp16(float16_t t) {
	return min(t, FP16_MAX);
}

/**/

/**/

#define INV_RADIUS (1.0f / l.limit)
#define INNER_ANGLE /*l.position.y*/0.785398
#define OUTER_ANGLE /*l.position.z*/1.0472

/**
 * @param dist Distance between light and position
 * @param imd Inverse Max Distance between ligth and position
 */
float linear_falloff(float dist, float imd) {
	return 1.0 - saturate(dist * imd);
}

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
vec3 light_spotlight(GlslLight l, vec3 position) {
	vec3 diff = l.position.xyz - position;
	vec3 ltp = normalize(diff); // Normalize Difference = Light To Position = Direction

	float attenuation = 1.0f *
		spotlight_falloff(length(diff), INV_RADIUS) *
		spotlight_angle(ltp, l.direction.xyz, INNER_ANGLE, OUTER_ANGLE);
	return l.luminance.rgb * attenuation/* * ndl Which is already in BRDF */;
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
vec3 light_pointlight(GlslLight l, vec3 position) {
    return l.luminance.rgb * linear_falloff(length(l.position.xyz - position), 1.0f / l.limit);
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
vec3 light_directional(GlslLight l, vec3 normal) {
	// float ndl = clamp(dot(normal, normalize(l.direction)), 0.0, 1.0);
	return l.luminance.rgb/* * ndl Which is already in BRDF */;
}

/**/

/**/

float shadow_reproject_filter(vec4 position, vec2 offset, uint cascade, float bias) {

	const float ambient = 0.07;
	float shadow = 1.0f;

	if (-1.0f < position.z && position.z < 1.0f) {

		//vec2 st = position.st * 0.5f + 0.5f + offset;
		vec2 st = position.st + offset;
		float dist = texture(sampler2DArray(shadowDir, shadowDirSampler), vec3(st, cascade)).r;
		if (
			position.w > 0 && dist < position.z - bias
		) {
			shadow = ambient;
		}

	}

	return shadow;
}

float shadow_attenuation_pcf(vec4 position, uint cascade, float bias) {

	ivec2 dim = textureSize(sampler2DArray(shadowDir, shadowDirSampler), 0).xy;

	float scale = 1.0f;
	float dx = scale * 1.0f / float(dim.x);
	float dy = scale * 1.0f / float(dim.y);

	float sf = 0.0f;
	int samples = 0;
	int range = 1;

	for (int y = -range; y <= range; ++y) {
		for (int x = -range; x <= range; ++x) {
			sf += shadow_reproject_filter(position, vec2(dx * x, dy * y), cascade, bias);
			++samples;
		}
	}

	return sf / float(samples);
}

float shadow_attenuation(vec4 worldPosition, float ndl) {

	vec4 viewPosition = co.view * worldPosition.xyzw;

	uint cascade = 0;
	for (uint i = 0; i < SHADOW_CASCADE_COUNT - 1; ++i) {
		if (viewPosition.z < sso.shadows[0].cascadeDepth[i]) {
			cascade = i + 1;
		}
	}

	const mat4 tndcMat = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.5, 0.5, 0.0, 1.0
	);

	float bias = clamp(0.005f * tan(acos(ndl)), 0.0, 0.01);
	bias *= 1.0 / (sso.shadows[0].cascadeDepth[cascade] * -pow(0.25, float(cascade + 1) / 2.0));

	vec4 shadowPosition = (tndcMat * sso.shadows[0].cascadeViewProj[cascade]) * vec4(worldPosition.xyz, 1.0);

	return shadow_attenuation_pcf(shadowPosition / shadowPosition.w, cascade, bias);

	//return shadow_reproject_filter(shadowPosition / shadowPosition.w, vec2(0.0), cascade, bias);
	//return float(cascade) / 3.0f;
}

/**/

/**/

// ACES Filmic Tone Mapping
vec3 tonemap(vec3 src) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;

    return saturate((src * (a * src + b)) / (src * (c * src + d) + e));
}

// From http://filmicgames.com/archives/75
vec3 uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

/**
 * Normal Distribution Function
 */

/**
 * GGX-NDF - Throwbridge-Reitz
 *
 * @see https://github.com/google/filament/blob/main/shaders/src/brdf.fs#L54
 *
 *	${GGX-NDF}_{Throwbridge-Reitz} = {\alpha^2 \over \pi((N \cdot H)^2 (\alpha^2 - 1) + 1)^2}$
 */
float ggx_ndf_throw_reitz(float ndh, float rough) {

	float ap2 = (rough * rough) * (rough * rough);

	float denom0 = (ndh * ndh) * (ap2 - 1.0) + 1.0;
	float denom = PI * (denom0 * denom0);

	return ap2 / max(denom, epsilon_f32);
}

float16_t ggx_ndf_fp16(float16_t ndh, const f16vec3 n, const f16vec3 h, float16_t rough) {
	
	f16vec3 nxh = cross(n, h);
	float16_t a = ndh * rough;
	float16_t k = rough / (dot(nxh, nxh) + a * a);
	float16_t d = k * k * (float16_t(1.0) / float16_t(PI));

	return saturateFp16(d);
}

/**
 * Geometric Shadowing Function
 */

/**
 * GGX - Schlick, Smith
 *
 * @see https://github.com/google/filament/blob/main/shaders/src/brdf.fs
 *
 *	$GGX_{Smith} = G_1(L,N) G_1(V,N)$
 *	$GGX_{Schlick-Beckmann} = {N \cdot X \over (N \cdot X)(1 - k) + k}$
 *	$k = {\alpha \over 2}$ and $X = V or L$
 */
float ggx_schlick_smith(float ndl, float ndv, float rough) {

	float k = (rough * rough) / 2.0;

	float ggx_l = ndl / max((ndl * (1.0 - k) + k), epsilon_f32);
	float ggx_v = ndv / max((ndv * (1.0 - k) + k), epsilon_f32);

	return ggx_l * ggx_v;
}

/**
 * Fresnel Function - Schlick
 *
 *	$F_{Schlick} = F_0 + (1 - F_0)(1 - (V \cdot H))^5$
 *	$F_{Schlick_2} = F_0 + (F_{90} - F_0)(1 - (V \cdot H))^5$
 */
vec3 fresnel_sch(float ct, vec3 F0) {
	return F0 + (vec3(1.0) - F0) * pow(1.0 - ct, 5.0);
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

	return (lightScatter * viewScatter * fac) * (1.0 / PI);
}

float lambert_diff() {
	return 1.0 / PI;
}

/**
 *
 */
vec3 prefilteredReflection(vec3 R, float rough) {
	
	const float MAX_REFLECTION_LOD = 9.0;
	float lod = rough * MAX_REFLECTION_LOD;
	float lodF = floor(lod);
	float lodC = ceil(lod);

	vec3 lowerSample = textureLod(samplerCube(brdfPref, brdfPrefSampler), R, lodF).rgb;
	vec3 upperSample = textureLod(samplerCube(brdfPref, brdfPrefSampler), R, lodC).rgb;

	return mix(lowerSample, upperSample, lod - lodF);
}

/**
 * Tangent Space Normal Reconstruction
 */
vec3 getMappedNormal() {

	/**/
	vec4 sparseNormal = vec4(0.0);
	int normStatusCode = sparseTextureARB(sampler2D(brdfNormal, brdfNormalSampler), inLocalUv.st, sparseNormal);

	float normMinLod = 1.0f;
	while (!sparseTexelsResidentARB(normStatusCode) && normMinLod < 14.0f) {
		normStatusCode = sparseTextureClampARB(sampler2D(brdfNormal, brdfNormalSampler), inLocalUv.st, normMinLod, sparseNormal);
		normMinLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(normStatusCode)) {
		sparseNormal = vec4(0.5, 0.5, 1.0, 0.0);
	}

	return sparseNormal.rgb;
}

vec3 calcNormal() {
	
	vec3 pdx = dFdx(inWorldPosition.xyz);
	vec3 pdy = dFdy(inWorldPosition.xyz);

	vec3 st1 = dFdx(vec3(inLocalUv.st, 0.0));
	vec3 st2 = dFdy(vec3(inLocalUv.st, 0.0));

	vec3 T = (st2.t * pdx - st1.t * pdy) / (st1.s * st2.t - st2.s * st1.t);
	
	vec3 N = normalize(inWorldNormal.xyz);

	T = normalize(T - N * dot(N, T));

	vec3 B = normalize(cross(N, T));
	mat3 tbn = mat3(T, B, N);

	return normalize(tbn * (getMappedNormal() * 2.0 - 1.0));
}

/**
 *
 *	$L_o(x,w_o) = L_e(x,w_o) + \int_{\Omega} f_r(x,w_i,w_o) L_i(x,w_i)(w_i \cdot n) dw_i$
 *	$L_o(x,V) = L_e(x,V) + \int_{\Omega} f_r(x,L,V) L_i(x,L)(L \cdot N) dL$
 *
 *	$L_o(x,V) = L_e(x,V) + \sum_{n} f_r(x,L_n,V) L_i(x,L_n)(L_n \cdot N)$
 *
 *	$f_r(x,L_n,V) ~ BRDF = k_x f_{diffuse} + k_s f_{specular}$
 *	$k_d + k_s = 1$
 *
 *	$k_s = Fresnel$ -> $k_d = 1 - k_s$
 *
 *	$f_{diffuse} ~ f_{Lambert} = {color \over \pi} (L_n \cdot N)$
 *
 *	$f_{specular} ~ f_{Cook-Torrance} = {DGF \over 4(V \cdot N)(L \cdot N)}$
 *	-> $BRDF = k_x f_{diffuse} + \cancel{k_s} f_{specular}$
 *
 *	$D ~ D_{GGX-NDF/Throwbridge-Reitz}$
 *	$\alpha = {roughness}^2$
 *
 *	$G ~ G_{GGX/Schlick-Smith}$
 */

void main() {
	/**
	 * Get data from G-Buffer / Mrt-Buffer
	 */
	//vec3 albedo = pow(texture(brdfAlbedo, inLocalUv.st).rgb, vec3(2.2));
	vec3 albedo = texture(sampler2D(brdfAlbedo, brdfAlbedoSampler), inLocalUv.st).rgb;
	//vec3 normal = normalize(texture(brdfNormal, inLocalUv).xyz);
	vec3 normal = calcNormal();
	vec4 position = inWorldPosition;
	vec3 arm = texture(sampler2D(brdfArm, brdfArmSampler), inLocalUv.st).rgb;

	//
	vec3 color = vec3(0.0);

	/**
	 *
	 */
	vec3 copos = inverse(co.view)[3].xyz;
	vec3 vd = normalize(copos - position.xyz);
	float ndvd = saturate(dot(normal, vd));

	vec3 reflection = reflect(-vd, normal);

	/**
	 * PBR Shading
	 */

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, arm.b);

	float F90 = saturate(F0.r * 50.0);

	// reflectance equation
    vec3 Lo = vec3(0.0);

	for (int i = 0; i < sli.sceneLightCount; ++i) {
		// Current Light
		GlslLight L = sl.lights[i];

		// calculate per-light radiance
        vec3 ltp = normalize(L.position.xyz - position.xyz); // Light to Position
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
			ltp = normalize(-L.direction.xyz);
			h = normalize(vd + ltp);
			ndl = saturate(dot(normal, ltp));

			// Warning: Temporary
			//outShadingColor = vec4(vec3(shadow_attenuation(inWorldPosition)), 1.0);
			float shadow = shadow_attenuation(inWorldPosition, ndl);
			radiance *= shadow;
		}

		// precalculate values
        float ndh = saturate(dot(normal, h));
		float ldh = saturate(dot(ltp, h));
        float hdvd = max(dot(h, vd), 0.0);

		if (ndl > 0.0 && radiance.x > 0.0) {
			/**
			 * Specular BRDF
			 */
			float D = ggx_ndf_throw_reitz(ndh, arm.g); // Normal Distribution
			float G = ggx_schlick_smith(ndl, ndvd, arm.g); // Geometric Shadowing
			//vec3 F = fresnel_sch(ldh, F0, F90); // Fresnel Factor
			vec3 F = fresnel_sch(ldh, F0); // Fresnel Factor

			vec3 num = D * G * F;
			float denom = 4.0 * ndvd * ndl;

			/**
			 * Diffuse BRDF
			 */
			//float fD = lambert_diff();
			float fD = disney_diff(ndvd, ndl, ldh, arm.g);

			// add to outgoing radiance Lo
			// vec3 kS = F;
			// vec3 kD = vec3(1.0) - kS;

			vec3 dc = albedo.rgb * (1.0 - arm.b);
			vec3 kD = (vec3(1.0) - F) * dc;

			vec3 ct = num / max(denom, epsilon_f32);
			vec3 brdf = kD * fD + ct;
			Lo += brdf * radiance * ndl;
		}
	}

	// Ambient Lighting
	vec3 kS = fresnel_sch_rough(ndvd, F0, arm.g);
	vec3 kD = (1.0 - kS) * (1.0 - arm.b);

	vec2 brdf = texture(sampler2D(brdfLut, brdfLutSampler), vec2(ndvd, arm.g)).rg;
	vec3 preflec = prefilteredReflection(reflection, arm.g);
	vec3 irradiance = texture(samplerCube(brdfIrrad, brdfIrradSampler), normal).rgb;

	// Ambient :: Irradiance Diffuse
	vec3 irrDiff = irradiance * albedo.rgb;

	// Ambient :: Specular Reflect
	vec3 rspec = preflec * (kS * brdf.x + brdf.y);

	vec3 ambient = (kD * irrDiff + rspec) * arm.rrr;
	// vec3 ambient = irradiance * albedo.rgb /* * ao.r */ * 0.8;
	// vec3 ambient = (kD * irrDiff) * mrs.b;
	//vec3 ambient = vec3(0.001);

	// Composite
	color = ambient + Lo;

	// HDR Tonemapping
	//color = uncharted2Tonemap(color * EXPOSURE);
	//color = color * (1.f / uncharted2Tonemap(vec3(11.2f)));
	color = tonemap(color);

	// Gamma Correction
	color = pow(color, vec3(1.0 / 2.2));

	outShadingColor = vec4(color, 1.0);
}