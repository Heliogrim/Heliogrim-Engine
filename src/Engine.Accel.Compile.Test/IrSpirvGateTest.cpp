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
}
