
#include <defined>
#include <lighting.brdf>

@output (mode = forward/*, id = "color" */)
u8vec4 color;

@input (mode = bind, id = "material.albedo")
uniform sampler2DArray albedo;

@input (mode = bind, id = "material.normal")
uniform sampler2DArray normal;

@input (mode = bind, id = "material.arm")
uniform sampler2DArray arm;

/**/

float SampleAmbientOcclusion(uint8_t layer, vec2 uv) {
    return Sample(arm, vec3(uv, layer)).r;
}

float SampleRoughness(uint8_t layer, vec2 uv) {
    return Sample(arm, vec3(uv, layer)).g;
}

float SampleMetalness(uint8_t layer, vec2 uv) {
    return Sample(arm, vec3(uv, layer)).b;
}

/**/

void main() {
    
    color = ColorBrdf(
        GetWorldPosition(),
        GetWorldNormal(),
        Sample(albedo, vec3(GetLocalUV(), 0)),
        SampleAmbientOcclusion(0, GetLocalUV()),
        SampleRoughness(0, GetLocalUV()),
        SampleMetalness(0, GetLocalUV())
    );
}