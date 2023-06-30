#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_sparse_texture2 : enable
#extension GL_ARB_sparse_texture_clamp : enable

layout(early_fragment_tests) in;

/* === */

// Platform Source Stage

/*
layout in uniform $bind/material {
	uint $bind/material/albedo;
    uint $bind/material/normal;
    uint $bind/material/roughness;
    uint $bind/material/metalness;
    uint $bind/material/ao;
    uint $bind/material/alpha;
};
*/

layout uniform sampler2DArray bind/albedo;
layout uniform sampler2DArray bind/normal;
layout uniform sampler2DArray bind/arm;

// --

layout in vec4 $in/normal;
layout in vec4 $in/position;
layout in vec3 $in/uvm;

// --

layout out vec4 $out/position;
layout out vec3 $out/normal;
layout out vec4 $out/albedo;
layout out vec4 $out/arm;

/* === */


vec3 getMappedNormal() {

	#define uv_scale_factor 1.0

	/**/
	vec4 sparseNormal = vec4(0.0);
	int normStatusCode = sparseTextureARB(bind/normal, vec3($in/uvm.st * uv_scale_factor, 0.0), sparseNormal);

	float normMinLod = 1.0f;
	while (!sparseTexelsResidentARB(normStatusCode) && normMinLod < 14.0f) {
		normStatusCode = sparseTextureClampARB(bind/normal, vec3($in/uvm.st * uv_scale_factor, 0.0), normMinLod, sparseNormal);
		normMinLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(normStatusCode)) {
		sparseNormal = vec4(0.0, 1.0, 0.0, 0.0);
	}

	#undef uv_scale_factor

	return sparseNormal.rgb;
}

vec3 calcNormal() {
	
	vec3 pdx = dFdx($in/position.xyz);
	vec3 pdy = dFdy($in/position.xyz);

	vec3 st1 = dFdx(vec3($in/uvm.st, 0.0));
	vec3 st2 = dFdy(vec3($in/uvm.st, 0.0));

	vec3 T = (st2.t * pdx - st1.t * pdy) / (st1.s * st2.t - st2.s * st1.t);
	
	vec3 N = normalize($in/normal.xyz);

	T = normalize(T - N * dot(N, T));

	vec3 B = normalize(cross(N, T));
	mat3 tbn = mat3(T, B, N);

	return normalize(tbn * (getMappedNormal() * 2.0 - 1.0));
}

void main() {
	//$out/albedo = texture(mapped_albedo, vec3($in/uvm.st, 0.0));
	$out/normal = $in/normal.xyz;
	$out/position = $in/position;

	#define uv_scale_factor 1.0

	vec4 sparseAlbedo = vec4(0.0);
	int statusCode = sparseTextureARB(bind/albedo, vec3($in/uvm.st * uv_scale_factor, 0.0), sparseAlbedo);

	if (!sparseTexelsResidentARB(statusCode)) {
		statusCode = sparseTextureARB(bind/albedo, vec3($in/uvm.st * uv_scale_factor, 0.0), sparseAlbedo, -1.0f);
	}

	float minLod = 1.0f;
	while (!sparseTexelsResidentARB(statusCode) && minLod < 14.0f) {
		statusCode = sparseTextureClampARB(bind/albedo, vec3($in/uvm.st * uv_scale_factor, 0.0), minLod, sparseAlbedo);
		minLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(statusCode)) {
		sparseAlbedo = vec4(0.0);
	}

	$out/albedo = sparseAlbedo;

	/**/
	vec4 sparseNormal = vec4(0.0);
	int normStatusCode = sparseTextureARB(bind/normal, vec3($in/uvm.st * uv_scale_factor, 0.0), sparseNormal);

	if (!sparseTexelsResidentARB(normStatusCode)) {
		statusCode = sparseTextureARB(bind/normal, vec3($in/uvm.st * uv_scale_factor, 0.0), sparseNormal, -1.0f);
	}

	float normMinLod = 1.0f;
	while (!sparseTexelsResidentARB(normStatusCode) && normMinLod < 14.0f) {
		normStatusCode = sparseTextureClampARB(bind/normal, vec3($in/uvm.st * uv_scale_factor, 0.0), normMinLod, sparseNormal);
		normMinLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(normStatusCode)) {
		sparseNormal = vec4(0.0);
	}

	/**/
	$out/normal = calcNormal();

	/**/
	vec4 sparseArm = vec4(0.0);
	int armStatusCode = sparseTextureARB(bind/arm, vec3($in/uvm.st * uv_scale_factor, 0.0), sparseArm);

	if (!sparseTexelsResidentARB(armStatusCode)) {
		statusCode = sparseTextureARB(bind/arm, vec3($in/uvm.st * uv_scale_factor, 0.0), sparseArm, -1.0f);
	}

	float armMinLod = 1.0f;
	while (!sparseTexelsResidentARB(armStatusCode) && armMinLod < 14.0f) {
		armStatusCode = sparseTextureClampARB(bind/arm, vec3($in/uvm.st * uv_scale_factor, 0.0), armMinLod, sparseArm);
		armMinLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(armStatusCode)) {
		sparseArm = vec4(1.0);
		armMinLod = 15.0f;
	}

	$out/arm = vec4(sparseArm.rgb, 0.0);
}