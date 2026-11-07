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
}
