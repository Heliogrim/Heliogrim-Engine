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
	uint ao;
} material;
/*
*/
layout (set = 2, binding = 4) uniform sampler2DArray mapped_albedo;
layout (set = 2, binding = 5) uniform sampler2DArray mapped_normal;
layout (set = 2, binding = 6) uniform sampler2DArray mapped_roughness;
layout (set = 2, binding = 7) uniform sampler2DArray mapped_specular;
/*
*/

layout (location = 0) in vec4 fragPosition;
layout (location = 1) in vec4 fragNormal;
layout (location = 2) in vec3 fragUvm;

layout (location = 0) out vec4 out_albedo;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec4 out_position;
layout (location = 3) out vec4 out_mrs;

void main() {
	out_albedo = texture(mapped_albedo, vec3(fragUvm.st, 0.0));
	out_normal = fragNormal.xyz;
	out_position = fragPosition;
	out_mrs = vec4(
		0.0,
		texture(mapped_roughness, vec3(fragUvm.st, 0.0)).r,
		texture(mapped_specular, vec3(fragUvm.st, 0.0)).r,
		1.0
	);

	//out_albedo = fragPosition;

	#define uv_scale_factor 4.0

	vec4 sparseAlbedo = vec4(0.0);
	int statusCode = sparseTextureARB(mapped_albedo, vec3(fragUvm.st * uv_scale_factor, 0.0), sparseAlbedo);

	float minLod = 1.0f;
	while (!sparseTexelsResidentARB(statusCode) && minLod < 13.0f) {
		statusCode = sparseTextureClampARB(mapped_albedo, vec3(fragUvm.st * uv_scale_factor, 0.0), minLod, sparseAlbedo);
		minLod += 1.0f;
	}

	if (!sparseTexelsResidentARB(statusCode)) {
		sparseAlbedo = vec4(0.0);
	}

	out_albedo = sparseAlbedo;
}