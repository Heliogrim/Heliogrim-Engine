#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define PI 3.1415926535897932384626433832795
#define EXPOSURE 4.5f

struct light {
	vec3 color; // Intensity Light-Color ( lightColor * intensity )
	vec3 position;
	vec3 direction;
	float limit; // Distance limit for light
};

const light lights[5] = light[] (
    light (
        vec3(1.0, 1.0, 1.0),
        vec3(0.0, 0.0, 0.0),
        normalize(vec3(96.0, 96.0, 0.0) - vec3(64.0, 0.0, 64.0)),
        -1.0
    ),
    light (
        vec3(43.47, 41.31, 16.7),
        vec3(64.0, 0.5, 64.0),
        vec3(0.0),
        48.0f
    ),
    light (
        vec3(19.3, 17.1, 83.2),
        vec3(32.0, 2.0, 96.0),
        normalize(vec3(32.0, 2.0, 96.0) - vec3(64.0, 0.0, 64.0)),
        90.509667991878083123308078349421f
    ),
    light (
        vec3(63.0, 21.0, 23.0),
        vec3(96.0, 2.0, 96.0),
        normalize(vec3(96.0, 2.0, 96.0) - vec3(64.0, 0.0, 64.0)),
        90.509667991878083123308078349421f
    ),
    light (
        vec3(19.0, 73.0, 28.0),
        vec3(96.0, 2.0, 32.0),
        normalize(vec3(96.0, 2.0, 32.0) - vec3(64.0, 0.0, 64.0)),
        90.509667991878083123308078349421f
    )
);

layout (location = 0) in vec2 fragUv;
layout (location = 1) in vec3 fragColor;

layout (set = 0, binding = 1) uniform CameraObj {
    vec3 pos;
} co;

layout (set = 0, binding = 2) uniform sampler2D mrtPosition;
layout (set = 0, binding = 3) uniform sampler2D mrtNormal;
layout (set = 0, binding = 4) uniform sampler2D mrtAlbedo;
layout (set = 0, binding = 5) uniform sampler2D mrtMetalness;
layout (set = 0, binding = 6) uniform sampler2D mrtRoughness;
layout (set = 0, binding = 7) uniform sampler2D mrtAo;
layout (set = 0, binding = 8) uniform sampler2D mrtSpecular;
layout (set = 0, binding = 8) uniform sampler2D mrtDepth;

layout (location = 0) out vec4 outColor;

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
 * @param distance Distance between light and position
 * @param imd Inverse Max Distance between ligth ans position
 */
float spotlight_falloff(float distance, float imd) {
	float d2 = distance * distance;
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
	float fac = clamp(ldl * scale + off, 0.0, 1.0);
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
 */
float ndf_ggx(float ndh, float rough) {
	float a = rough * rough;
    float a2 = a * a;

    float denom = ((ndh * ndh) * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;

    return a2 / max(denom, 0.001f); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}

/**
 * Geometric Shadowing Function - Schlick, Smith
 *
 * @author Julius
 * @date 27.12.2020
 */
float geo_sch_smith(float ndv, float ndl, float rough) {
    float r = (rough + 1.0f);
    float k = (r * r) / 8.0f;

    float ggxv = ndv / (ndv * (1.0f - k) + k);
    float ggxl = ndl / (ndl * (1.0f - k) + k);

    return ggxv * ggxl;
}

/**
 * Fresnel Function - Schlick
 *
 * @author Julius
 * @date 27.12.2020
 */
vec3 fresnel_sch(float ct, vec3 F0) {
	return F0 + (1.0f - F0) * pow(1.0f - ct, 5.0f);
}

void main() {
	/**
	 * Get data from G-Buffer / Mrt-Buffer
	 */
	vec3 position = texture(mrtPosition, fragUv).xyz;
	vec3 normal = normalize(texture(mrtNormal, fragUv).xzy);
	vec4 albedo = texture(mrtAlbedo, fragUv);
	float metalness = texture(mrtMetalness, fragUv).r;
	float roughness = texture(mrtRoughness, fragUv).r;
	float ao = texture(mrtAo, fragUv).r;
	float specular = texture(mrtSpecular, fragUv).r;

    //normal = normalize(vec3(0.0, 1.0, 0.0));

	/**
	 *
	 */
	vec3 vd = normalize(co.pos - position);

	/**
	 * PBR Shading
	 */
	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04f); 
    F0 = mix(F0, albedo.rgb, metalness);

    // reflectance equation
    vec3 Lo = vec3(0.0f);
    for(int i = 0; i < 5; ++i) {
        light cl = lights[i];

        // calculate per-light radiance
        vec3 ltp = normalize(cl.position - position); // Light to Postition
        vec3 h = normalize(vd + ltp);

        vec3 radiance;
        float ndl;

        if (cl.limit > 0.0001f) {
            
            if ((cl.direction.x + cl.direction.y + cl.direction.z) < 0.0001f) {
                radiance = light_pointlight(cl, position);

            } else {
                radiance = light_spotlight(cl, position);

            }

            ndl = clamp(dot(normal, ltp), 0.0, 1.0);

        } else {
            radiance = light_directional(cl, normal);
            ndl = clamp(dot(normal, cl.direction), 0.0, 1.0);
        }

        // precalculate values
        float ndvd = clamp(dot(normal, vd), 0.0f, 1.0f);
        float ndh = clamp(dot(normal, h), 0.0f, 1.0f);
        float hdvd = clamp(dot(h, vd), 0.0f, 1.0f);

        if (ndl > 0.0f) {

            // Cook-Torrance BRDF
            float D = ndf_ggx(ndh, roughness); // Normal Distribution
            float G = geo_sch_smith(ndvd, ndl, roughness); // Geometric Shadowing
            vec3 F = fresnel_sch(hdvd, F0); // Fresnel Factor

            vec3 specular = (D * G * F) / max(4.0f * ndl * ndvd, 0.001f); // prevent divide by zero for ndv=0.0 or ndl=0.0

            vec3 kD = (vec3(1.0f) - F) * (1.0f - metalness);

            // add to outgoing radiance Lo
            Lo += (kD * albedo.rgb / PI + specular) * radiance * ndl;
        }
    }

    // ambient lighting
    vec3 irradiance = vec3(0.03f);
    vec3 ambient = irradiance * albedo.rgb * ao;

    // Combine
    vec3 color = ambient + Lo;

	/**
     * Color Correction
     */

     // HDR Tonemapping
	//color = color / (color + vec3(1.0));
    color = tonemap(color * EXPOSURE);
    color = color * ( 1.0f / tonemap(vec3(11.2f)) );

    // Bypass Skybox Color
    if (albedo.a <= 0.001f) {
        color = albedo.rgb;
    }

    // Gamma Correction
	color = pow(color, vec3(1.0f / 2.2f));

    outColor = vec4(color, 1.0f);
}