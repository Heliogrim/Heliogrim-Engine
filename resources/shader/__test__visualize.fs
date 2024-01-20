
#extension GL_EXT_shader_explicit_arithmetic_types : enable

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 inLocalUv;

layout (set = 0, binding = 0) uniform texture2D visual;
layout (set = 1, binding = 0) uniform sampler visualSampler;

layout (location = 0) out vec4 out_color;

void main() {
	out_color = vec4(inLocalUv.st, 0.0, 1.0);
	out_color = vec4(1.0 - texture(sampler2D(visual, visualSampler), inLocalUv.st).rrr, 1.0);
}