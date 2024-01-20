
#include <defined>

#define EXPOSURE 4.5f

@input (mode = forward, id = "color")
u8vec4 PrevColor;

@output (mode = forward, id = "color")
u8vec4 NextColor;

/**/

// http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 Tonemapping(vec3 color) {

    const float A = 0.15f;
    const float B = 0.50f;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.02f;
    const float F = 0.30f;

    return ( ( color * ( A * color + C * B ) + D * E ) / ( color * ( A * color + B ) + D * F ) ) - E / F;
}

vec3 ApplyTonemapping(vec3 color) {
    color = Tonemapping(color * EXPOSURE);
    return color * ( 1.0 / Tonemapping(vec3(11.2)) );
}

vec3 ApplyGamma(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

/**/

void main() {
    NextColor = vec4(
        ApplyGamma(ApplyTonemapping(PrevColor.rgb)),
        PrevColor.a
    );
}