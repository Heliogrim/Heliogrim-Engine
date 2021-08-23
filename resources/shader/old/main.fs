#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
//#extension GL_ARB_shader_image_load_store : enable

#define const_ambient 0.1
#define const_scale 0.75
#define const_shadow 0.25

#define MATERIAL_COUNT 16
#define SHADOW_MAP_CASCADES 4

#define _texId ambient.a
#define _alpha mapping.a
#define _normal mapping.r
#define _diffuse mapping.g
#define _specular mapping.b

//layout (early_fragment_tests) in;

layout (set = 0, binding = 1) uniform sampler2DArray albedoArray;
layout (set = 0, binding = 2) uniform sampler2DArray shadowMap;

layout (set = 0, binding = 3) uniform sampler2DArray diffuseArray;
layout (set = 0, binding = 4) uniform sampler2DArray specularArray;
layout (set = 0, binding = 5) uniform sampler2DArray normalArray;
layout (set = 0, binding = 6) uniform sampler2DArray alphaArray;
//layout (set = 0, binding = 9) uniform sampler2DArray roughMetalArray;
//layout (set = 0, binding = 10) uniform sampler2DArray displaceArray;

struct AmbientLight// | base alignment = 16 + 16 + 16 = 64 | stride = 64
{
	vec4 direction;// direction | base alignment = 16 | offset = 0
	vec4 diffuse;// diffuse | base alignment = 16 | offset = 16
	vec4 specular;// specular | base alignment = 16 | offset = 32
	vec4 ambient;// ambient | base alignment = 16 | offset = 48
};

struct Material// | base alignment = 16 + 16 + 16 + 16 + 4 = 68 | stride = 80
{
	/* Ambient : RGB - Color; A - Texture Id */
	vec4 ambient;// ambient | base alignment = 16 | offset = 0
	vec4 diffuse;// diffuse | base alignment = 16 | offset = 16
	vec4 specular;// specular color | base alignment = 16 | offset = 32
	vec4 mapping;// mapping | base alignment = 16 | offset = 48
	float shininess;// shininess | base alignment = 4 | offset = 64
};

layout (std140, set = 0, binding = 7) uniform UniformBufferObjFrag// | base alignment = 64 + 16 + 16 + 4 + 64 = 164 | stride = 176
{
	AmbientLight light;// base alignment = 64
	vec4 camPos;// base alignment = 16 | offset = 64
	vec4 skyColor;// base alignment = 16 | offset = 80
	vec4 splits;// base alignment = 4 | offset = 96
	mat4 casMVP[SHADOW_MAP_CASCADES];// base alignment = 64 | offset = 100
} ufrag;

layout (std430, set = 0, binding = 8) buffer MaterialStorage
{
	Material data[];
} materials;

layout (location = 0) in vec3 fragColor;// color of vertex
layout (location = 1) in vec3 fragUVW;// texture uvw
layout (location = 2) in vec3 fragNormal;// normal at position
layout (location = 3) in vec3 fragVert;// surface position
layout (location = 4) in vec3 fragView;
layout (location = 5) in float visibility;


layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outDiffuse;
layout (location = 4) out vec4 outSpecular;

void main()
{
	int index = int(fragUVW.z);

	vec4 color = texture(albedoArray, vec3(fragUVW.xy, materials.data[index]._texId));
	vec4 alpha = texture(alphaArray, vec3(fragUVW.xy, materials.data[index]._alpha));

	if (color.a < 0.5)
		discard;
	
	outPosition = vec4(fragVert.z, fragVert.y, fragVert.x, 1.0);

	// TODO: calc normal in tangent space
	vec3 N = normalize(fragNormal);
	if (materials.data[index]._normal >= 0) {
		vec4 normal = texture(normalArray, vec3(fragUVW.xy, materials.data[index]._normal));
		outNormal = vec4(N.z, N.y, N.x, 1.0) + normal;
	} else {
		outNormal = vec4(N.z, N.y, N.x, 1.0);
	}

	outAlbedo = color;

	if (materials.data[index]._diffuse >= 0) {
		outDiffuse = texture(diffuseArray, vec3(fragUVW.xy, materials.data[index]._diffuse));
	} else {
		outDiffuse = vec4(0.0);
	}

	if (materials.data[index]._specular >= 0) {
		outSpecular = texture(specularArray, vec3(fragUVW.xy, materials.data[index]._specular));
	} else {
		outSpecular = vec4(0.0);
	}
}