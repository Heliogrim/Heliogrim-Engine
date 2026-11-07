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
}
