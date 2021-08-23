#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec3 fragUvm;

struct material {
	uint albedo;
	uint normal;
	uint metalness;
	uint roughness;
	uint ao;
	uint alpha;
};

/*
layout (set = 1, binding = 6) buffer MaterialsObj {
	material unwrap[];
} mo;
*/

layout (set = 1, binding = 7) uniform sampler2DArray albedoMap;
layout (set = 1, binding = 8) uniform sampler2DArray normalMap;
layout (set = 1, binding = 9) uniform sampler2DArray metalnessMap;
layout (set = 1, binding = 10) uniform sampler2DArray roughnessMap;
layout (set = 1, binding = 11) uniform sampler2DArray aoMap;
layout (set = 1, binding = 12) uniform sampler2DArray alphaMap;
layout (set = 1, binding = 13) uniform sampler2DArray specularMap;
layout (set = 1, binding = 14) uniform sampler2DArray displaceMap;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outMetalness;
layout (location = 4) out vec4 outRoughness;
layout (location = 5) out vec4 outAo;
layout (location = 6) out vec4 outSpecular;

void main() {
	
	/**
	 * Get original material data
	 */
	/*
	material mat = mo.unwrap[int(fragUvm.z)];

	outPosition = vec4(fragPosition, 1.0);
	outNormal = vec4(fragNormal, 1.0);
	outAlbedo = texture(albedoMap, vec3(fragUvm.xy, mat.albedo));
	outMetalness = texture(metalnessMap, vec3(fragUvm.xy, mat.metalness));
	outRoughness = texture(roughnessMap, vec3(fragUvm.xy, mat.roughness));
	outAo = texture(aoMap, vec3(fragUvm.xy, mat.ao));
	*/

	outPosition = vec4(fragPosition, 1.0);

	int idx = 1;
	outAlbedo = texture(albedoMap, vec3(fragUvm.xy, idx));
	outAlbedo = vec4(outAlbedo.rgb, 1.0);
}