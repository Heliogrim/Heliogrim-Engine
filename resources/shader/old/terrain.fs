#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define const_ambient 0.1
#define const_scale 0.75
#define const_shadow 0.25

#define MATERIAL_COUNT 16
#define SHADOW_MAP_CASCADES 4

layout (binding = 1) uniform sampler2D blendTexture;
layout (binding = 2) uniform sampler2DArray texArray;
// layout (binding = 3) uniform sampler2DArray shadowMap;

struct AmbientLight// | base alignment = 16 + 16 + 16 = 64 | stride = 64
{
	vec4 direction;// direction | base alignment = 16 | offset = 0
	vec4 diffuse;// diffuse | base alignment = 16 | offset = 16
	vec4 specular;// specular | base alignment = 16 | offset = 32
	vec4 ambient;// ambient | base alignment = 16 | offset = 48
};

struct Material// | base alignment = 16 + 16 + 16 + 4 = 52 | stride = 64
{
	vec4 ambient;// ambient | base alignment = 16 | offset = 0
	vec4 diffuse;// diffuse | base alignment = 16 | offset = 16
	vec4 specular;// specular color | base alignment = 16 | offset = 32
	vec4 mapping;// mapping | base alignment = 16 | offset = 48
	float shininess;// shininess | base alignment = 4 | offset = 64
};

/*
layout (std430, set = 0, binding = 3) buffer MaterialStorage
{
	Material data[];
} materials;
*/

// Layout inputs
layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragUV;
layout (location = 2) in vec3 fragNormal;
layout (location = 3) in vec3 fragVert;
layout (location = 4) in vec3 fragView;
layout (location = 5) in float visibility;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outDiffuse;
layout (location = 4) out vec4 outSpecular;

void main()
{
	// Blend Mapping of Terrain
	vec4 blendMapColor = texture(blendTexture, fragUV.xy);

	//vec2 tiledCoords = fragUV.xy * 512.0;
	vec2 tiledCoords = fragUV.xy * 128;

	vec4 backTC = texture(texArray, vec3(tiledCoords.xy, 0.0)) * (1.0 - blendMapColor.r - blendMapColor.g - blendMapColor.b);
	vec4 rTC = texture(texArray, vec3(tiledCoords.xy, 1.0)) * blendMapColor.r;
	vec4 gTC = texture(texArray, vec3(tiledCoords.xy, 2.0)) * blendMapColor.g;
	vec4 bTC = texture(texArray, vec3(tiledCoords.xy, 3.0)) * blendMapColor.b;

	outPosition = vec4(fragVert.z, fragVert.y, fragVert.x, 1.0);
	outNormal = vec4(fragNormal.z, fragNormal.y, fragNormal.z, 0.0);
	outAlbedo = (backTC + rTC + gTC + bTC);
	
	outDiffuse = vec4(0.0);
	outSpecular = vec4(0.0);
}