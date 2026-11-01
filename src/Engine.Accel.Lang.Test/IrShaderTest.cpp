#include "pch.h"

#include <Engine.Accel.Lang/Emitter/GlslEmitter.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>

#include "IrShaderFixtures.hpp"

using namespace hg;
using namespace hg::engine::accel::lang;
using namespace hg::engine::accel::lang::ir;

namespace AccelLang {
	TEST(PreDepthShaderTest, InterfaceEqualsOldAnnotations) {
		const auto fragment = shaderAssetModule("depth/predepth.fs.asl", EntryKind::eFragment);
		const auto interfaceOf = computeInterface(fragment);

		ASSERT_EQ(interfaceOf.size(), 1uz);
		const auto& depth = interfaceOf.front();
		EXPECT_EQ(depth.id, "depth");
		EXPECT_EQ(depth.mode, BindMode::eForward);
		EXPECT_FALSE(depth.input);
		EXPECT_TRUE(depth.implicit);
		EXPECT_EQ(fragment.types[depth.type.index].kind, TypeKind::eScalar);
		EXPECT_EQ(fragment.types[depth.type.index].scalar, ScalarKind::eFloat);
		EXPECT_EQ(fragment.types[depth.type.index].bits, 32u);

		const auto vertex = shaderAssetModule("depth/predepth.vs.asl", EntryKind::eVertex);
		EXPECT_TRUE(computeInterface(vertex).empty());
	}

	TEST(PreDepthShaderTest, EmitsNonEmptyVertexGlsl) {
		const auto vertex = shaderAssetModule("depth/predepth.vs.asl", EntryKind::eVertex);
		const auto layout = assignLayout(computeInterface(vertex), vertex.types, {});

		Language target {};
		const auto result = GlslEmitter {}.emit(vertex, layout, target);
		ASSERT_TRUE(result.diagnostics.empty()) << result.diagnostics.front().message;
		ASSERT_EQ(target.text.size(), 1uz);

		const auto& text = target.text.front().data;
		EXPECT_TRUE(text.starts_with("#version 460\n")) << text;
		EXPECT_TRUE(text.contains("gl_VertexIndex")) << text;
		EXPECT_TRUE(text.contains("gl_Position")) << text;
	}
}
