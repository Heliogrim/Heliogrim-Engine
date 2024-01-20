
#define PI 3.1415926535897932384626433832795

#extension GL_EXT_multiview : enable

/*
*/
layout (set = 1, binding = 0) uniform textureCube mapped_env;
layout (set = 2, binding = 0) uniform sampler mapped_env_sampler;
/*
*/

layout (location = 0) in vec3 fragUvw;

layout (location = 0) out vec4 out_color;

layout (constant_id = 0) const float DELTA_PHI = 2.0 * PI / 180.0;
layout (constant_id = 1) const float DELTA_THETA = 0.5 * PI / 64.0;

void main() {
	
	vec3 N = normalize(fragUvw);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));

	// Reorthogonalize
	up = cross(N, right);

	const float PI2 = PI * 2.0;
	const float PIH = PI * 0.5;

	vec3 color = vec3(0.0);
	uint samples = 0u;

	for (float phi = 0.0; phi < PI2; phi += DELTA_PHI) {
		for (float theta = 0.0; theta < PIH; theta += DELTA_THETA) {

			vec3 tv = cos(phi) * right + sin(phi) * up;
			vec3 sv = cos(theta) * N + sin(theta) * tv;

			color += texture(samplerCube(mapped_env, mapped_env_sampler), sv).rgb * cos(theta) * sin(theta);
			++samples;
		}
	}

	/*
	 * Layers: [ X+ X- Y+ Y- Z+ Z- ]
	 */

	out_color = vec4(PI * color / float(samples), 1.0);
}