
#extension GL_EXT_shader_explicit_arithmetic_types : enable

#extension GL_ARB_separate_shader_objects : enable

#define EXPOSURE 4.5f

layout (location = 0) in vec3 fragUvw;

layout (set = 1, binding = 0) uniform textureCube skybox;
layout (set = 2, binding = 0) uniform sampler skyboxSampler;

layout (location = 0) out vec4 outAlbedo;

vec3 saturate(vec3 x) {
    return clamp(x, 0.0, 1.0);
}

// ACES Filmic Tone Mapping
vec3 tonemap(vec3 src) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;

    return saturate((src * (a * src + b)) / (src * (c * src + d) + e));
}

void main() {
	vec3 color = texture(samplerCube(skybox, skyboxSampler), fragUvw.xyz).rgb;
	color = tonemap(color);
	outAlbedo = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
}