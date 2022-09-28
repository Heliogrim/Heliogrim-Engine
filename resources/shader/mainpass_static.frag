#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_sparse_texture2 : enable
#extension GL_ARB_sparse_texture_clamp : enable

layout(early_fragment_tests) in;

/*
*/
layout (set = 2, binding = 3) uniform MaterialUbo {
	uint diffuse;
	uint normal;
	uint roughness;
	uint metalness;
	uint ao;
	uint alpha;
} material;
/*
*/
layout (set = 2, binding = 4) uniform sampler2DArray mapped_albedo;
layout (set = 2, binding = 5) uniform sampler2DArray mapped_normal;
layout (set = 2, binding = 6) uniform sampler2DArray mapped_roughness;
layout (set = 2, binding = 7) uniform sampler2DArray mapped_metalness;
layout (set = 2, binding = 8) uniform sampler2DArray mapped_specular;
layout (set = 2, binding = 9) uniform sampler2DArray mapped_alpha;
layout (set = 2, binding = 10) uniform sampler2DArray mapped_ao;
/*
*/

layout (location = 0) in vec4 fragPosition;
layout (location = 1) in vec4 fragNormal;
layout (location = 2) in vec3 fragUvm;

layout (location = 0) out vec4 out_albedo;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec4 out_position;
layout (location = 3) out vec4 out_mrs;


vec3 getMappedNormal() {

	#define uv_scale_factor 1.0

	/**/
	vec4 sparseNormal = vec4(0.0);
	int normStatusCode = sparseTextureARB(mapped_normal, vec3(fragUvm.st * uv_scale_factor, 0.0), sparseNormal);

	float normMinLod = 1.0f;
	while (!sparseTexelsResidentARB(normStatusCode) && normMinLod < 14.0f) {
		normStatusCode = sparseTextureClampARB(mapped_normal, vec3(fragUvm.st * uv_scale_factor, 0.0), normMinLod, sparseNormal);
		normMinLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(normStatusCode)) {
		sparseNormal = vec4(0.0, 1.0, 0.0, 0.0);
	}

	#undef uv_scale_factor

	return sparseNormal.rgb;
}

vec3 calcNormal() {
	
	vec3 pdx = dFdx(fragPosition.xyz);
	vec3 pdy = dFdy(fragPosition.xyz);

	vec3 st1 = dFdx(vec3(fragUvm.st, 0.0));
	vec3 st2 = dFdy(vec3(fragUvm.st, 0.0));

	vec3 T = (st2.t * pdx - st1.t * pdy) / (st1.s * st2.t - st2.s * st1.t);
	
	vec3 N = normalize(fragNormal.xyz);

	T = normalize(T - N * dot(N, T));

	vec3 B = normalize(cross(N, T));
	mat3 tbn = mat3(T, B, N);

	return normalize(tbn * (getMappedNormal() * 2.0 - 1.0));
}

void main() {
	//out_albedo = texture(mapped_albedo, vec3(fragUvm.st, 0.0));
	out_normal = fragNormal.xyz;
	out_position = fragPosition;

	#define uv_scale_factor 1.0

	vec4 sparseAlbedo = vec4(0.0);
	int statusCode = sparseTextureARB(mapped_albedo, vec3(fragUvm.st * uv_scale_factor, 0.0), sparseAlbedo);

	if (!sparseTexelsResidentARB(statusCode)) {
		sparseTextureARB(mapped_albedo, vec3(fragUvm.st * uv_scale_factor, 0.0), sparseAlbedo, -1.0f);
	}

	float minLod = 1.0f;
	while (!sparseTexelsResidentARB(statusCode) && minLod < 14.0f) {
		statusCode = sparseTextureClampARB(mapped_albedo, vec3(fragUvm.st * uv_scale_factor, 0.0), minLod, sparseAlbedo);
		minLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(statusCode)) {
		sparseAlbedo = vec4(0.0);
	}

	out_albedo = sparseAlbedo;

	/**/
	vec4 sparseNormal = vec4(0.0);
	int normStatusCode = sparseTextureARB(mapped_normal, vec3(fragUvm.st * uv_scale_factor, 0.0), sparseNormal);

	if (!sparseTexelsResidentARB(normStatusCode)) {
		sparseTextureARB(mapped_normal, vec3(fragUvm.st * uv_scale_factor, 0.0), sparseNormal, -1.0f);
	}

	float normMinLod = 1.0f;
	while (!sparseTexelsResidentARB(normStatusCode) && normMinLod < 14.0f) {
		normStatusCode = sparseTextureClampARB(mapped_normal, vec3(fragUvm.st * uv_scale_factor, 0.0), normMinLod, sparseNormal);
		normMinLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(normStatusCode)) {
		sparseNormal = vec4(0.0);
	}

	/**/
	out_normal = calcNormal();

	/**/
	vec4 sparseRough = vec4(0.0);
	int roughStatusCode = sparseTextureARB(mapped_roughness, vec3(fragUvm.st * uv_scale_factor, 0.0), sparseRough);

	if (!sparseTexelsResidentARB(roughStatusCode)) {
		sparseTextureARB(mapped_normal, vec3(fragUvm.st * uv_scale_factor, 0.0), sparseRough, -1.0f);
	}

	float roughMinLod = 1.0f;
	while (!sparseTexelsResidentARB(roughStatusCode) && roughMinLod < 14.0f) {
		roughStatusCode = sparseTextureClampARB(mapped_roughness, vec3(fragUvm.st * uv_scale_factor, 0.0), roughMinLod, sparseRough);
		roughMinLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(roughStatusCode)) {
		sparseRough = vec4(1.0);
		roughMinLod = 15.0f;
	}

	vec4 sparseMetal = vec4(0.0);
	int metalStatusCode = sparseTextureARB(mapped_metalness, vec3(fragUvm.st * uv_scale_factor, 0.0), sparseMetal);

	if (!sparseTexelsResidentARB(metalStatusCode)) {
		sparseTextureARB(mapped_normal, vec3(fragUvm.st * uv_scale_factor, 0.0), sparseMetal, -1.0f);
	}

	float metalMinLod = 1.0f;
	while (!sparseTexelsResidentARB(metalStatusCode) && metalMinLod < 14.0f) {
		metalStatusCode = sparseTextureClampARB(mapped_metalness, vec3(fragUvm.st * uv_scale_factor, 0.0), metalMinLod, sparseMetal);
		metalMinLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(metalStatusCode)) {
		sparseMetal = vec4(0.0);
	}

	vec4 sparseAo = vec4(1.0);
	int aoStatusCode = sparseTextureARB(mapped_ao, vec3(fragUvm.st * uv_scale_factor, 0.0), sparseAo);

	float aoMinLod = 1.0f;
	while (!sparseTexelsResidentARB(aoStatusCode) && aoMinLod < 14.0f) {
		aoStatusCode = sparseTextureClampARB(mapped_ao, vec3(fragUvm.st * uv_scale_factor, 0.0), aoMinLod, sparseAo);
		aoMinLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(aoStatusCode)) {
		sparseAo = vec4(1.0);
	}

	out_mrs = vec4(sparseMetal.r, sparseRough.r, sparseAo.r, 0.0);
}