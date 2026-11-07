#include "pch.h"

#include "IrSpirvGate.hpp"

using namespace hg::engine::accel::lang::ir;

namespace AccelCompile {
	TEST(IrSpirvGateTest, PreDepthVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("depth/predepth.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, PreDepthFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("depth/predepth.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, ShadowVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("shadow/shadowDir.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, ShadowFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("shadow/shadowDir.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, UiVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("ui/ui.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, UiFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("ui/ui.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, PostProcessVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("postprocessing/pp.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, PostProcessFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("postprocessing/pp.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, VisualizeVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("visualize/visualize.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, VisualizeFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("visualize/visualize.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, TriTestVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("test/render.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, TriTestFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("test/render.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, BrdfLutVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/brdfLut.gen.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, BrdfLutFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/brdfLut.gen.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, SkyVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("skybox/sky.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, SkyFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("skybox/sky.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, CubeGenVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/cubeGen.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, BrdfIrradFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/brdfIrrad.gen.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, BrdfPrefFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/brdfPref.gen.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, BrdfVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/brdf.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, BrdfFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/brdf.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, RejectsBrokenGlsl) {
		EXPECT_FALSE(glslCompiles("#version 460\nvoid main(){ gl_Position = vec4(0); }", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, ConstantsSample) {
		EXPECT_TRUE(
			compilesToSpirv(
				"const SCALE: f32 = 0.5;\n"
				"const TABLE: f32vec3[2] = f32vec3[2](f32vec3(1.0), f32vec3(2.0));\n"
				"@spec (id = \"sample-count\") const SAMPLES: u32 = 4u;\n"
				"@input (mode = bind, id = \"m\", class = uniform, packing = std140) block m { f32vec4 a; };\n"
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn main() {\n"
				"	let acc = 0.0;\n"
				"	for (let i = 0u; i < SAMPLES; ++i) { acc += SCALE; }\n"
				"	color = f32vec4(TABLE[1] * acc, 1.0) + m.a;\n"
				"}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, ReferencesSample) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = bind, id = \"lights\", class = storage, access = read_write) block lights { f32[] lights; };\n"
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn F(a: &mut f32, b: &f32, c: f32) { a = b + c; }\n"
				"fn main() {\n	u32 i = 1; let l = &mut lights.lights[i];\n	F(l, 1.0, 2.0);\n	const k = l;\n	color = f32vec4(k);\n}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, ControlFlowSample) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn radical(bits_: u32) -> f32 {\n"
				"	let bits = bits_;\n"
				"	bits = (bits << 16u) | (bits >> 16u);\n"
				"	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);\n"
				"	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);\n"
				"	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);\n"
				"	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);\n"
				"	return f32(bits) * 2.3283064365386963e-10;\n"
				"}\n"
				"fn main() {\n"
				"	let acc = 0.0;\n"
				"	let n = 0u;\n"
				"	for (let i = 0u; i < 8u; ++i) { acc += radical(i); }\n"
				"	while (acc > 1.0) { acc -= 1.0; n++; if (n > 4u) { break; } if (acc > 100.0) { continue; } }\n"
				"	acc *= 0.5;\n"
				"	let r = acc > 0.5 ? acc : 1.0;\n"
				"	color = f32vec4(r, f32(n ^ 3u), f32(~n & 1u), 1.0);\n"
				"	if (r < 0.0) { discard; }\n"
				"}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, MathSample) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn main() {\n"
				"	let v = f32vec3(0.25, 0.5, 0.75);\n"
				"	let m = f32mat4(1.0);\n"
				"	let n = f32mat3(m);\n"
				"	let t = transpose(inverse(n)) * normalize(clamp(v, 0.1, 0.9));\n"
				"	let a = mix(v, t, 0.5) + reflect(v, f32vec3(0.0, 1.0, 0.0)) + cross(v, t);\n"
				"	let b = dot(a, v) + length(a) + distance(a, v) + pow(abs(a.x), 2.0) + smoothstep(0.0, 1.0, a.y);\n"
				"	color = f32vec4(floor(a) + fract(a) + dFdx(a) + fwidth(a), sqrt(b) + sin(b) + max(b, 0.5));\n"
				"}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, TextureSample) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = bind, id = \"t2\", class = texture) texture2d t2;\n"
				"@input (mode = bind, id = \"ta\", class = texture) texture2darray ta;\n"
				"@input (mode = bind, id = \"tc\", class = texture) texturecube tc;\n"
				"@input (mode = bind, id = \"smp\", class = sampler) sampler smp;\n"
				"@input (mode = forward, id = \"uv\") f32vec2 uv;\n"
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn main() {\n"
				"	let uv3 = f32vec3(uv, 0.0);\n"
				"	let a = f32vec4(0.0);\n"
				"	let b = f32vec4(0.0);\n"
				"	let acc = sample(t2, smp, uv) + sample(ta, smp, uv3) + sample(tc, smp, uv3);\n"
				"	acc += sample_lod(t2, smp, uv, 1.0) + sample_lod(ta, smp, uv3, 1.0) + sample_lod(tc, smp, uv3, 1.0);\n"
				"	let sz = texture_size(t2, smp, 0) + texture_size(tc, smp, 0) + s32vec2(texture_size(ta, smp, 0).xy);\n"
				"	let r = sample_sparse(t2, smp, uv, a) + sample_sparse(ta, smp, uv3, a) + sample_sparse(tc, smp, uv3, a);\n"
				"	let q = sample_sparse_clamp(t2, smp, uv, 1.0, b) + sample_sparse_clamp(ta, smp, uv3, 1.0, b) + sample_sparse_clamp(tc, smp, uv3, 1.0, b);\n"
				"	if (sparse_resident(r) && sparse_resident(q)) { acc += a + b; }\n"
				"	color = acc + f32vec4(f32(sz.x));\n"
				"}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, ForWithOmittedInitAndStep) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn main() {\n"
				"	let acc = 0.0;\n"
				"	let i = 0u;\n"
				"	for (; i < 4u; ) { acc += 0.25; i++; }\n"
				"	for (let j = 0u; j < 2u; ) { acc += 0.5; j++; }\n"
				"	for (; acc < 8.0; ++acc) { }\n"
				"	color = f32vec4(acc);\n"
				"}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, ComputeReduction) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = bind, id = \"data\", class = storage, access = read_write) block data { f32[] values; };\n"
				"workgroup scratch: f32[64];\n"
				"@compute (local_size = [64, 1, 1])\n"
				"fn main() {\n"
				"	let lid = local_invocation_index;\n"
				"	scratch[lid] = data.values[global_invocation_id.x];\n"
				"	barrier();\n"
				"	for (let stride = 32u; stride > 0u; stride = stride / 2u) {\n"
				"		if (lid < stride) { scratch[lid] += scratch[lid + stride]; }\n"
				"		barrier();\n"
				"	}\n"
				"	if (lid == 0u) { data.values[workgroup_id.x] = scratch[0]; }\n"
				"}\n",
				EntryKind::eCompute
			)
		);
	}

	TEST(IrSpirvGateTest, ComputeImageBlur) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = bind, id = \"src\", class = image, access = read) image2d<rgba16f> src;\n"
				"@input (mode = bind, id = \"dst\", class = image, access = write) image2d<rgba16f> dst;\n"
				"@compute (local_size = [8, 8, 1])\n"
				"fn main() {\n"
				"	let p = s32vec2(global_invocation_id.xy);\n"
				"	let acc = f32vec4(0.0);\n"
				"	for (let dx = -1; dx <= 1; dx++) {\n"
				"		acc += image_load(src, p + s32vec2(dx, 0));\n"
				"	}\n"
				"	image_store(dst, p, acc / 3.0);\n"
				"}\n",
				EntryKind::eCompute
			)
		);
	}
}
