
#extension GL_EXT_shader_explicit_arithmetic_types : enable

#extension GL_ARB_separate_shader_objects : enable

#define PI 3.1415926535897932384626433832795
#define EXPOSURE 4.5f

layout (location = 0) in vec2 inLocalUv;

layout (set = 0, binding = 0) uniform texture2D color_image;
layout (set = 1, binding = 0) uniform sampler color_sampler;

layout (location = 0) out vec4 out_color;

// http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 tonemap(vec3 src) {
    const float A = 0.15f;
    const float B = 0.50f;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.02f;
    const float F = 0.30f;

    return ( ( src * ( A * src + C * B ) + D * E ) / ( src * ( A * src + B ) + D * F ) ) - E / F;
}

void main() {
	//out_color = vec4(inLocalUv.st, 0.0, 1.0);
	//out_color = vec4(texture(color_sampler, inLocalUv.st).rgb, 1.0);

	vec3 color = texture(sampler2D(color_image, color_sampler), inLocalUv.st).rgb;

	// HDR Tonemapping
	color = tonemap(color * EXPOSURE);
    color = color * ( 1.0 / tonemap(vec3(11.2)) );

	// Gamma Correction
	color = pow(color, vec3(1.0 / 2.2));

	out_color = vec4(color, 1.0);
}