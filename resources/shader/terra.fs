#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_texture_query_lod : enable

layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec2 fragUv;

layout (set = 1, binding = 4) uniform sampler2D blendingMap;

struct material {
	uint albedo;
	uint normal;
	uint metalness;
	uint roughness;
	uint ao;
	uint alpha;
};

layout (set = 2, binding = 6) buffer MaterialsObj {
	material unwrap[];
} mo;

layout (set = 2, binding = 7) uniform sampler2DArray albedoMap;
layout (set = 2, binding = 8) uniform sampler2DArray normalMap;
layout (set = 2, binding = 9) uniform sampler2DArray metalnessMap;
layout (set = 2, binding = 10) uniform sampler2DArray roughnessMap;
layout (set = 2, binding = 11) uniform sampler2DArray aoMap;
layout (set = 2, binding = 12) uniform sampler2DArray alphaMap;
layout (set = 2, binding = 13) uniform sampler2DArray specularMap;
layout (set = 2, binding = 14) uniform sampler2DArray displaceMap;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outMetalness;
layout (location = 4) out vec4 outRoughness;
layout (location = 5) out vec4 outAo;
layout (location = 6) out vec4 outSpecular;

void main() {

	/**
	 *
	 */
	vec2 tiled = fragUv * 32.0;
	
	/**
	 * Get Blend Mapping
	 */
	vec4 blending = texture(blendingMap, fragUv);

	float black = (((1.0 - blending.r) - blending.g) - blending.b);

	/**
	 * Get original material data
	 */
	material blackm = mo.unwrap[1];
	material redm = mo.unwrap[2];
	material greenm = mo.unwrap[3];
	material bluem = mo.unwrap[4];

	blackm.normal = 0;
	blackm.albedo = 0;
	blackm.roughness = 0;
	blackm.ao = 0;
	redm.normal = 1;
	redm.albedo = 1;
	redm.roughness = 1;
	redm.ao = 1;
	greenm.normal = 2;
	greenm.albedo = 2;
	greenm.roughness = 2;
	greenm.ao = 2;
	bluem.normal = 3;
	bluem.albedo = 3;
	bluem.roughness = 3;
	bluem.ao = 3;
	
	outPosition = vec4(fragPosition, 1.0);

	// Height Correction
	float dmv0 = texture(displaceMap, vec3(tiled, blackm.normal)).r;
	float dmv1 = texture(displaceMap, vec3(tiled, redm.normal)).r;
	float dmv2 = texture(displaceMap, vec3(tiled, greenm.normal)).r;
	float dmv3 = texture(displaceMap, vec3(tiled, bluem.normal)).r;

	float bv1 = blending.r;
	float bv2 = blending.g;
	float bv3 = blending.b;

	float td = 0.2;
	float mda = max(
		max(dmv0 * black + black, dmv1 * bv1 + bv1),
		max(dmv2 * bv2 + bv2, dmv3 * bv3 + bv3)
	) - td;

	float fm0 = max(dmv0 * black + black - mda, 0.0);
	float fm1 = max(dmv1 * bv1 + bv1 - mda, 0.0);
	float fm2 = max(dmv2 * bv2 + bv2 - mda, 0.0);
	float fm3 = max(dmv3 * bv3 + bv3 - mda, 0.0);
	float fmr = fm0 + fm1 + fm2 + fm3;

	outNormal = (
		texture(normalMap, vec3(tiled, blackm.normal)) * fm0 +
		texture(normalMap, vec3(tiled, redm.normal)) * fm1 +
		texture(normalMap, vec3(tiled, greenm.normal)) * fm2 +
		texture(normalMap, vec3(tiled, bluem.normal)) * fm3
	) / fmr;

	outAlbedo = (
		texture(albedoMap, vec3(tiled, blackm.albedo)) * fm0 +
		texture(albedoMap, vec3(tiled, redm.albedo)) * fm1 +
		texture(albedoMap, vec3(tiled, greenm.albedo)) * fm2 +
		texture(albedoMap, vec3(tiled, bluem.albedo)) * fm3
	) / fmr;

	outRoughness = (
		texture(roughnessMap, vec3(tiled, blackm.roughness)) * fm0 +
		texture(roughnessMap, vec3(tiled, redm.roughness)) * fm1 +
		texture(roughnessMap, vec3(tiled, greenm.roughness)) * fm2 +
		texture(roughnessMap, vec3(tiled, bluem.roughness)) * fm3
	) / fmr;

	outAo = (
		texture(aoMap, vec3(tiled, blackm.ao)) * fm0 +
		texture(aoMap, vec3(tiled, redm.ao)) * fm1 +
		texture(aoMap, vec3(tiled, greenm.ao)) * fm2 +
		texture(aoMap, vec3(tiled, bluem.ao)) * fm3
	) / fmr;

	outSpecular = (
		texture(specularMap, vec3(tiled, blackm.ao)) * fm0 +
		texture(specularMap, vec3(tiled, redm.ao)) * fm1 +
		texture(specularMap, vec3(tiled, greenm.ao)) * fm2 +
		texture(specularMap, vec3(tiled, bluem.ao)) * fm3
	) / fmr;
}