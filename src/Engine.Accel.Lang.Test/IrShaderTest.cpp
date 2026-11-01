#include "pch.h"

#include <Engine.Accel.Lang/Emitter/GlslEmitter.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
#include <Engine.Accel.Lang/Ir/Lower.hpp>

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

	TEST(ShadowShaderTest, InterfaceEqualsOldAnnotations) {
		const auto vertex = shaderAssetModule("shadow/shadowDir.vs.asl", EntryKind::eVertex);
		const auto entries = computeInterface(vertex);

		struct Expected {
			StringView id;
			BindMode mode;
			BindClass cls;
		};
		// inputs sorted by id
		constexpr std::array expected {
			Expected { "block", BindMode::eBind, BindClass::ePushConstant },
			Expected { "model", BindMode::eBind, BindClass::eStorageBuffer },
			Expected { "vertex-position", BindMode::eForward, BindClass::eNone },
			Expected { "view", BindMode::eBind, BindClass::eStorageBuffer }
		};
		ASSERT_EQ(entries.size(), expected.size());
		for (auto i = 0uz; i < expected.size(); ++i) {
			EXPECT_EQ(entries[i].id, expected[i].id);
			EXPECT_EQ(entries[i].mode, expected[i].mode) << entries[i].id;
			EXPECT_EQ(entries[i].cls, expected[i].cls) << entries[i].id;
			EXPECT_TRUE(entries[i].input) << entries[i].id;
		}

		const auto& position = vertex.types[entries[2].type.index];
		EXPECT_EQ(position.kind, TypeKind::eVector);
		EXPECT_EQ(position.n, 3u);

		const auto fragment = shaderAssetModule("shadow/shadowDir.fs.asl", EntryKind::eFragment);
		const auto outputs = computeInterface(fragment);
		ASSERT_EQ(outputs.size(), 1uz);
		EXPECT_EQ(outputs.front().id, "depth");
		EXPECT_FALSE(outputs.front().input);
		EXPECT_TRUE(outputs.front().implicit);
		EXPECT_EQ(outputs.front().mode, BindMode::eForward);
	}

	TEST(ShadowShaderTest, BindingNumbersComeFromLayoutOnly) {
		const auto vertex = shaderAssetModule("shadow/shadowDir.vs.asl", EntryKind::eVertex);
		const auto layout = assignLayout(computeInterface(vertex), vertex.types, {});

		// both storage blocks are laid out in set 0, in id order
		ASSERT_EQ(layout.slots.size(), 2uz);
		EXPECT_EQ(layout.slots[0].id, "model");
		EXPECT_EQ(layout.slots[1].id, "view");
		for (auto i = 0u; i < layout.slots.size(); ++i) {
			EXPECT_EQ(layout.slots[i].set, 0u);
			EXPECT_EQ(layout.slots[i].binding, i);
		}

		const DenseMap<String, u32> offsets { { "vertex-position", 0u } };
		const auto lowered = lowerToIntermediate(vertex, layout, Dialect::eVulkanGlsl460, offsets);
		ASSERT_TRUE(lowered.has_value()) << lowered.error().front().message;

		const auto& text = lowered->lang.text.front().data;
		EXPECT_TRUE(text.contains("layout(set = 0, binding = 0) buffer")) << text;
		EXPECT_TRUE(text.contains("layout(set = 0, binding = 1) buffer")) << text;
		EXPECT_FALSE(text.contains("set = 1")) << text;
	}

	TEST(ShadowShaderTest, PushBlockSizeIsFourBytes) {
		const auto vertex = shaderAssetModule("shadow/shadowDir.vs.asl", EntryKind::eVertex);
		const auto layout = assignLayout(computeInterface(vertex), vertex.types, {});

		ASSERT_EQ(layout.push.size(), 1uz);
		EXPECT_EQ(layout.push.front().id, "block");
		EXPECT_EQ(layout.push.front().offset, 0u);
		EXPECT_EQ(layout.push.front().size, 4u);
	}
}
