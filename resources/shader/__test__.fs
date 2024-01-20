//#version 460 core

#extension GL_EXT_shader_explicit_arithmetic_types : enable

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_sparse_texture2 : enable
#extension GL_ARB_sparse_texture_clamp : enable

#define HG_NAMED_MESH_TYPE_STATIC 1
#define HG_NAMED_MESH_TYPE_SKELETAL 2

#define VS_TO_FRAG_WORLD_POSITION 5701
#define VS_TO_FRAG_LOCAL_NORMAL 5702
#define VS_TO_FRAG_LOCAL_UV 5703

#define HG_MAT_BRDF_TEXTURE_SET 6501
#define HG_MAT_BRDF_TEXTURE_BIND_ALBEDO 6801
#define HG_MAT_BRDF_TEXTURE_BIND_NORMAL 6802
#define HG_MAT_BRDF_TEXTURE_BIND_ARM 6803

#define HG_DEFERRED_BRDF_ALDEBO 7301
#define HG_DEFERRED_BRDF_NORMAL 7302
#define HG_DEFERRED_BRDF_ARM 7303
#define HG_DEFERRED_BRDF_POSITION 7304

#define __UV_SCALE_FACTOR 1.0

#ifdef HG_SCTX_DEPTH_ONLY
	/**/
	layout(early_fragment_tests) in;
	/**/
	void main() {
	}
	/**/
#endif

/**/
/**/
/**/

#ifdef HG_SCTX_MATERIAL_PASS
	/*  */
	layout (set = HG_MAT_BRDF_TEXTURE_SET, binding = HG_MAT_BRDF_TEXTURE_BIND_ALBEDO) uniform sampler2DArray brdfAlbedo;
	layout (set = HG_MAT_BRDF_TEXTURE_SET, binding = HG_MAT_BRDF_TEXTURE_BIND_NORMAL) uniform sampler2DArray brdfNormal;
	layout (set = HG_MAT_BRDF_TEXTURE_SET, binding = HG_MAT_BRDF_TEXTURE_BIND_ARM) uniform sampler2DArray brdfArm;
	/**/
	layout (location = VS_TO_FRAG_WORLD_POSITION) in f32vec4 inWorldPosition;
	layout (location = VS_TO_FRAG_LOCAL_NORMAL) in f32vec4 inLocalNormal;
	layout (location = VS_TO_FRAG_LOCAL_UV) in f32vec3 inLocalUv;
	/**/
	layout (location = HG_DEFERRED_BRDF_ALDEBO) out vec4 outBrdfAlbedo;
	layout (location = HG_DEFERRED_BRDF_NORMAL) out vec3 outBrdfNormal;
	layout (location = HG_DEFERRED_BRDF_ARM) out vec3 outBrdfArm;
	layout (location = HG_DEFERRED_BRDF_POSITION) out f32vec4 outBrdfPosition;
	/*  */
	vec3 getMappedNormal() {

		/**/
		vec4 sparseNormal = vec4(0.0);
		int normStatusCode = sparseTextureARB(brdfNormal, vec3(inLocalUv.st * __UV_SCALE_FACTOR, 0.0), sparseNormal);

		float normMinLod = 1.0f;
		while (!sparseTexelsResidentARB(normStatusCode) && normMinLod < 14.0f) {
			normStatusCode = sparseTextureClampARB(brdfNormal, vec3(inLocalUv.st * __UV_SCALE_FACTOR, 0.0), normMinLod, sparseNormal);
			normMinLod += 1.0f;
		}

		if (!sparseTexelsResidentARB(normStatusCode)) {
			sparseNormal = vec4(0.0, 1.0, 0.0, 0.0);
		}

		return sparseNormal.rgb;
	}

	vec3 calcNormal() {
		
		vec3 pdx = dFdx(inWorldPosition.xyz);
		vec3 pdy = dFdy(inWorldPosition.xyz);

		vec3 st1 = dFdx(vec3(inLocalUv.st, 0.0));
		vec3 st2 = dFdy(vec3(inLocalUv.st, 0.0));

		vec3 T = (st2.t * pdx - st1.t * pdy) / (st1.s * st2.t - st2.s * st1.t);
		
		vec3 N = normalize(inLocalNormal.xyz);

		T = normalize(T - N * dot(N, T));

		vec3 B = normalize(cross(N, T));
		mat3 tbn = mat3(T, B, N);

		return normalize(tbn * (getMappedNormal() * 2.0 - 1.0));
	}

	vec4 sampleBrdfAlbedo() {

		vec4 albedo = vec4(0.0);
		int statusCode = sparseTextureARB(brdfAlbedo, vec3(inLocalUv.st * __UV_SCALE_FACTOR, 0.0), albedo);

		if (!sparseTexelsResidentARB(statusCode)) {
			sparseTextureARB(brdfAlbedo, vec3(inLocalUv.st * __UV_SCALE_FACTOR, 0.0), albedo, -1.0f);
		}

		float minLod = 1.0f;
		while (!sparseTexelsResidentARB(statusCode) && minLod < 14.0f) {
			statusCode = sparseTextureClampARB(brdfAlbedo, vec3(inLocalUv.st * __UV_SCALE_FACTOR, 0.0), minLod, albedo);
			minLod += 1.0f;
		}

		if (!sparseTexelsResidentARB(statusCode)) {
			albedo = vec4(0.0);
		}

		return albedo;
	}

	vec3 sampleBrdfArm() {

		vec4 arm = vec4(0.0);
		int statusCode = sparseTextureARB(brdfArm, vec3(inLocalUv.st * __UV_SCALE_FACTOR, 0.0), arm);

		if (!sparseTexelsResidentARB(statusCode)) {
			sparseTextureARB(brdfArm, vec3(inLocalUv.st * __UV_SCALE_FACTOR, 0.0), arm, -1.0f);
		}

		float minLod = 1.0f;
		while (!sparseTexelsResidentARB(statusCode) && minLod < 14.0f) {
			statusCode = sparseTextureClampARB(brdfArm, vec3(inLocalUv.st * __UV_SCALE_FACTOR, 0.0), minLod, arm);
			minLod += 1.0f;
		}

		if (!sparseTexelsResidentARB(statusCode)) {
			arm = vec4(0.0);
		}

		return arm.xyz;
	}

	void main() {
		outBrdfAlbedo = sampleBrdfAlbedo();
		outBrdfNormal = calcNormal();
		outBrdfArm = sampleBrdfArm();
		outBrdfPosition = inWorldPosition;
	}
	/*  */
#endif