
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable

layout (push_constant) uniform PushConsts {
	layout (offset = 0) vec2 translate;
	layout (offset = 8) vec2 scale;
} meta;

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in vec3 in_uvm;
layout (location = 3) in uint8_t layer;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec3 fragUvm;

void main() {

	fragColor = in_color;
	fragUvm = in_uvm;

	gl_Position = vec4(in_position.xy * meta.scale + meta.translate, (float(layer) / float(255)), 1);
}