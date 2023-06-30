#version 450

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

layout (set = 2, binding = 0) uniform sampler2DArray bind_albedo;
layout (set = 3, binding = 0) uniform sampler2DArray bind_normal;
layout (set = 4, binding = 0) uniform sampler2DArray bind_arm;

// --

layout (location = 1) in vec4 __in_normal;
layout (location = 0) in vec4 __in_position;
layout (location = 2) in vec3 __in_uvm;

// --

layout (location = 0) out vec4 __out_position;
layout (location = 1) out vec3 __out_normal;
layout (location = 2) out vec4 __out_albedo;
layout (location = 3) out vec4 __out_arm;

/* === */


vec3 getMappedNormal() {

	#define uv_scale_factor 1.0

	/**/
	vec4 sparseNormal = vec4(0.0);
	int normStatusCode = sparseTextureARB(bind_normal, vec3(__in_uvm.st * uv_scale_factor, 0.0), sparseNormal);

	float normMinLod = 1.0f;
	while (!sparseTexelsResidentARB(normStatusCode) && normMinLod < 14.0f) {
		normStatusCode = sparseTextureClampARB(bind_normal, vec3(__in_uvm.st * uv_scale_factor, 0.0), normMinLod, sparseNormal);
		normMinLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(normStatusCode)) {
		sparseNormal = vec4(0.0, 1.0, 0.0, 0.0);
	}

	#undef uv_scale_factor

	return sparseNormal.rgb;
}

vec3 calcNormal() {
	
	vec3 pdx = dFdx(__in_position.xyz);
	vec3 pdy = dFdy(__in_position.xyz);

	vec3 st1 = dFdx(vec3(__in_uvm.st, 0.0));
	vec3 st2 = dFdy(vec3(__in_uvm.st, 0.0));

	vec3 T = (st2.t * pdx - st1.t * pdy) / (st1.s * st2.t - st2.s * st1.t);
	
	vec3 N = normalize(__in_normal.xyz);

	T = normalize(T - N * dot(N, T));

	vec3 B = normalize(cross(N, T));
	mat3 tbn = mat3(T, B, N);

	return normalize(tbn * (getMappedNormal() * 2.0 - 1.0));
}

void main() {
	//__out_albedo = texture(mapped_albedo, vec3(__in_uvm.st, 0.0));
	__out_normal = __in_normal.xyz;
	__out_position = __in_position;

	#define uv_scale_factor 1.0

	vec4 sparseAlbedo = vec4(0.0);
	int statusCode = sparseTextureARB(bind_albedo, vec3(__in_uvm.st * uv_scale_factor, 0.0), sparseAlbedo);

	if (!sparseTexelsResidentARB(statusCode)) {
		sparseTextureARB(bind_albedo, vec3(__in_uvm.st * uv_scale_factor, 0.0), sparseAlbedo, -1.0f);
	}

	float minLod = 1.0f;
	while (!sparseTexelsResidentARB(statusCode) && minLod < 14.0f) {
		statusCode = sparseTextureClampARB(bind_albedo, vec3(__in_uvm.st * uv_scale_factor, 0.0), minLod, sparseAlbedo);
		minLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(statusCode)) {
		sparseAlbedo = vec4(0.0);
	}

	__out_albedo = sparseAlbedo;

	/**/
	vec4 sparseNormal = vec4(0.0);
	int normStatusCode = sparseTextureARB(bind_normal, vec3(__in_uvm.st * uv_scale_factor, 0.0), sparseNormal);

	if (!sparseTexelsResidentARB(normStatusCode)) {
		sparseTextureARB(bind_normal, vec3(__in_uvm.st * uv_scale_factor, 0.0), sparseNormal, -1.0f);
	}

	float normMinLod = 1.0f;
	while (!sparseTexelsResidentARB(normStatusCode) && normMinLod < 14.0f) {
		normStatusCode = sparseTextureClampARB(bind_normal, vec3(__in_uvm.st * uv_scale_factor, 0.0), normMinLod, sparseNormal);
		normMinLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(normStatusCode)) {
		sparseNormal = vec4(0.0);
	}

	/**/
	__out_normal = calcNormal();

	/**/
	vec4 sparseArm = vec4(0.0);
	int armStatusCode = sparseTextureARB(bind_arm, vec3(__in_uvm.st * uv_scale_factor, 0.0), sparseArm);

	if (!sparseTexelsResidentARB(armStatusCode)) {
		sparseTextureARB(bind_arm, vec3(__in_uvm.st * uv_scale_factor, 0.0), sparseArm, -1.0f);
	}

	float armMinLod = 1.0f;
	while (!sparseTexelsResidentARB(armStatusCode) && armMinLod < 14.0f) {
		armStatusCode = sparseTextureClampARB(bind_arm, vec3(__in_uvm.st * uv_scale_factor, 0.0), armMinLod, sparseArm);
		armMinLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(armStatusCode)) {
		sparseArm = vec4(1.0);
		armMinLod = 15.0f;
	}

	__out_arm = vec4(sparseArm.rgb, 0.0);
}
