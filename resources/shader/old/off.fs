#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
//#extension GL_ARB_shader_image_load_store : enable

#define const_ambient 0.0025
#define const_scale 0.75
#define const_shadow 0.25

#define MATERIAL_COUNT 16
#define SHADOW_MAP_CASCADES 4

#define _matId albedo.a
#define _albedo texture(samplerAlbedo, fragUV)
#define _normal texture(samplerNormal, fragUV).rgb

//layout (early_fragment_tests) in;

layout (set = 0, binding = 1) uniform sampler2D samplerPosition;
layout (set = 0, binding = 2) uniform sampler2D samplerNormal;
layout (set = 0, binding = 3) uniform sampler2D samplerAlbedo;
layout (set = 0, binding = 4) uniform sampler2D samplerDiffuse;
layout (set = 0, binding = 5) uniform sampler2D samplerSpecular;

struct AmbientLight// | base alignment = 16 + 16 + 16 = 64 | stride = 64
{
	vec4 direction;// direction | base alignment = 16 | offset = 0
	vec4 diffuse;// diffuse | base alignment = 16 | offset = 16
	vec4 specular;// specular | base alignment = 16 | offset = 32
	vec4 ambient;// ambient | base alignment = 16 | offset = 48
};

struct Material// | base alignment = 16 + 16 + 16 + 4 = 52 | stride = 64
{
	/* Ambient : RGB - Color; A - Texture Id */
	vec4 ambient;// ambient | base alignment = 16 | offset = 0
	vec4 diffuse;// diffuse | base alignment = 16 | offset = 16
	vec4 specular;// specular color | base alignment = 16 | offset = 32
	float shininess;// shininess | base alignment = 4 | offset = 48
};

layout (std140, set = 0, binding = 6) uniform UniformBufferObjFrag// | base alignment = 64 + 16 + 16 + 4 + 64 = 164 | stride = 176
{
	AmbientLight light;// base alignment = 64
	vec4 camPos;// base alignment = 16 | offset = 64
	vec4 skyColor;// base alignment = 16 | offset = 80
	vec4 splits;// base alignment = 4 | offset = 96
	mat4 casMVP[SHADOW_MAP_CASCADES];// base alignment = 64 | offset = 100
} ufrag;

/*
layout (std430, set = 0, binding = 7) buffer MaterialStorage
{
	Material data[];
} materials;
*/

layout (location = 0) in vec2 fragUV;

layout (location = 0) out vec4 outColor;

void main()
{
	vec3 pos = texture(samplerPosition, fragUV).rgb;
	vec3 N = normalize(_normal);
	vec4 albedo = _albedo;

	vec3 V = normalize(ufrag.camPos.xyz - pos);

	vec3 color = ufrag.light.ambient.rgb * albedo.rgb;

	{
		int index = int(_matId);

		vec3 L = normalize(ufrag.light.direction.xyz);// normalize(light.position - pos)
		float DP = dot(N, -L);

		// attenuation
		float attenu = 1.0F;

		// diffuse lighting
		vec3 diffuse = attenu * ufrag.light.diffuse.rgb * albedo.rgb * max(0.0, DP);

		// specular lighting
		vec3 specular;

		/*
		if (materials.data[index].shininess == 0.0 || DP < 0.0)
		{
			specular = vec3(0.0);
		} else
		{
			vec3 R = reflect(-L, N);
			float coef = dot(R, V);

			specular = attenu * ufrag.light.specular.rgb * materials.data[index].specular.rgb * pow(max(0.0, coef), materials.data[index].shininess);
		}
		*/

		color += (diffuse/* + specular*/);
	}

	if (length(_normal) == 0) {
		outColor = vec4(albedo.rgb, 1.0);
	} else {
		outColor = vec4(color, 1.0);
	}

	outColor = vec4(texture(samplerAlbedo, fragUV).rgb, 1.0);

	vec3 test = texture(samplerAlbedo, fragUV).rgb;
	outColor = vec4(test, 1.0);
}