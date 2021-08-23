#version 450

struct light {
	vec3 color; // Intensity Light-Color ( lightColor * intensity )
	vec3 position;
	vec3 direction;
	float limit; // Distance limit for light
};

/**
 * https://google.github.io/filament/Filament.md#lighting/imagebasedlights
 * 
 * [Light]   | 10am       | 12pm       | 05:30pm
 *
 * Sky + Sun | 120.000 lx | 130.000 lx | 90.000 lx
 * Sky       |  20.000 lx |  25.000 lx |  9.000 lx
 * Sun       | 100.000 lx | 105.000 lx | 81.000 lx
 *
 * Table 12: Illuminance values in lx (a full moon has an illuminance of 1 lx)
 */

/**
 * .
 *
 * @author Julius
 * @date 26.12.2020
 *
 * @param l Light
 * @param normal Normal at Position
 */
vec3 light_directional(light l, vec3 normal) {
	float ndl = clamp(dot(normal, normalize(-l.direction)), 0.0, 1.0);
	return l.color * ndl;
}