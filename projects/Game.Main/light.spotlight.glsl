#version 450

#define INV_RADIUS l.limit
#define INNER_ANGLE /*l.position.y*/45.0
#define OUTER_ANGLE /*l.position.z*/60.0

struct light {
	vec3 color; // Intensity Light-Color ( lightColor * intensity )
	vec3 position;
	vec3 direction;
	float limit; // Distance limit for light
};

/**
 * .
 *
 * @author Julius
 * @date 26.12.2020
 *
 * @param distance Distance between light and position
 * @param md Max Distance between ligth ans position
 */
float spotlight_falloff(float distance, float md) {
	float d2 = distance * distance;
	float fac = d2 * md * md;
	float sfac = max(1.0 - fac * fac, 0.0);
	return (sfac * sfac) / max(d2, 1e-4);
}

/**
 * .
 *
 * @author Julius
 * @date 26.12.2020
 *
 * @param ltp Light To Position
 * @param ld Light Direction
 * @param inner Inner-Angle of Spot in radians
 * @param outer Outer-Angle of Spot in radians
 */
float spotlight_angle(vec3 ltp, vec3 ld, float inner, float outer) {
	
	float co = cos(outer);
	float scale = 1.0 / max(cos(inner) - co, 1e-4);
	float off = -co * scale;

	float ldl = dot(normalize(-ld), ltp);
	float fac = clamp(ldl * scale + off, 0.0, 1.0);
	return fac * fac;
}

/**
 * .
 *
 * @author Julius
 * @date 26.12.2020
 *
 * @param l Light
 * @param position Position
 * @param normal Normal as Position
 */
vec3 light_spotlight(light l, vec3 position, vec3 normal) {
	vec3 diff = l.position - position;

	vec3 ltp = normalize(diff); // Normalize Difference = Light To Position = Direction
	float ndl = clamp(dot(normal, ltp), 0.0, 1.0);

	float attenuation = 1.0f *
		spotlight_falloff(length(diff), INV_RADIUS) *
		spotlight_angle(ltp, l.direction, INNER_ANGLE, OUTER_ANGLE);
	return l.color * attenuation * ndl;
}