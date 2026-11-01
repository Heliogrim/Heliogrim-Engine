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

	namespace {
		[[nodiscard]] const InterfaceEntry* find(ref<const Vector<InterfaceEntry>> entries_, StringView id_, bool input_) {
			const auto it = std::ranges::find_if(
				entries_,
				[&](ref<const InterfaceEntry> e_) { return e_.id == id_ && e_.input == input_; }
			);
			return it != entries_.end() ? &*it : nullptr;
		}
	}

	TEST(UiShaderTest, InterfaceCoversOldAnnotations) {
		const auto vertex = shaderAssetModule("ui/ui.vs.asl", EntryKind::eVertex);
		const auto fragment = shaderAssetModule("ui/ui.fs.asl", EntryKind::eFragment);
		const auto vertexEntries = computeInterface(vertex);
		const auto fragmentEntries = computeInterface(fragment);

		struct Expected {
			StringView id;
			bool input;
			BindMode mode;
			BindClass cls;
			bool implicit;
		};
		constexpr std::array vertexExpected {
			Expected { "block", true, BindMode::eBind, BindClass::ePushConstant, false },
			Expected { "vertex-position", true, BindMode::eForward, BindClass::eNone, false },
			Expected { "vertex-color", true, BindMode::eForward, BindClass::eNone, false },
			Expected { "vertex-uv", true, BindMode::eForward, BindClass::eNone, false },
			Expected { "vertex-layer", true, BindMode::eForward, BindClass::eNone, false }
		};
		constexpr std::array fragmentExpected {
			Expected { "ui-image", true, BindMode::eBind, BindClass::eTexture, false },
			Expected { "ui-image-sampler", true, BindMode::eBind, BindClass::eSampler, false },
			Expected { "color", false, BindMode::eForward, BindClass::eNone, false },
			Expected { "depth", false, BindMode::eForward, BindClass::eNone, true }
		};

		for (const auto& e : vertexExpected) {
			const auto* entry = find(vertexEntries, e.id, e.input);
			ASSERT_NE(entry, nullptr) << e.id;
			EXPECT_EQ(entry->mode, e.mode) << e.id;
			EXPECT_EQ(entry->cls, e.cls) << e.id;
		}
		for (const auto& e : fragmentExpected) {
			const auto* entry = find(fragmentEntries, e.id, e.input);
			ASSERT_NE(entry, nullptr) << e.id;
			EXPECT_EQ(entry->mode, e.mode) << e.id;
			EXPECT_EQ(entry->cls, e.cls) << e.id;
			EXPECT_EQ(entry->implicit, e.implicit) << e.id;
		}

		/* 8-bit forward data is read as float, the stored format comes from the pass */
		const auto& color = vertex.types[find(vertexEntries, "vertex-color", true)->type.index];
		EXPECT_EQ(color.kind, TypeKind::eVector);
		EXPECT_EQ(color.scalar, ScalarKind::eFloat);
		EXPECT_EQ(color.n, 4u);
		const auto& layer = vertex.types[find(vertexEntries, "vertex-layer", true)->type.index];
		EXPECT_EQ(layer.kind, TypeKind::eScalar);
		EXPECT_EQ(layer.scalar, ScalarKind::eFloat);
	}

	TEST(UiShaderTest, PushLayoutIsTranslateScale) {
		const auto vertex = shaderAssetModule("ui/ui.vs.asl", EntryKind::eVertex);
		const auto entries = computeInterface(vertex);
		const auto layout = assignLayout(entries, vertex.types, {});

		ASSERT_EQ(layout.push.size(), 1uz);
		EXPECT_EQ(layout.push.front().id, "block");
		EXPECT_EQ(layout.push.front().offset, 0u);
		EXPECT_EQ(layout.push.front().size, 16u);

		const auto offsets = std430Offsets(vertex.types, find(entries, "block", true)->type);
		ASSERT_EQ(offsets.size(), 2uz);
		EXPECT_EQ(offsets[0], 0u);
		EXPECT_EQ(offsets[1], 8u);
	}

	TEST(UiShaderTest, ForwardedVaryingsMatchByIdAcrossStages) {
		const auto vertex = shaderAssetModule("ui/ui.vs.asl", EntryKind::eVertex);
		const auto fragment = shaderAssetModule("ui/ui.fs.asl", EntryKind::eFragment);
		const auto vertexEntries = computeInterface(vertex);
		const auto fragmentEntries = computeInterface(fragment);

		for (const auto id : { "ui-color"sv, "ui-uv"sv }) {
			const auto* out = find(vertexEntries, id, false);
			const auto* in = find(fragmentEntries, id, true);
			ASSERT_NE(out, nullptr) << id;
			ASSERT_NE(in, nullptr) << id;
			EXPECT_EQ(out->mode, BindMode::eForward) << id;
			EXPECT_EQ(in->mode, BindMode::eForward) << id;

			const auto& outType = vertex.types[out->type.index];
			const auto& inType = fragment.types[in->type.index];
			EXPECT_EQ(outType.kind, inType.kind) << id;
			EXPECT_EQ(outType.scalar, inType.scalar) << id;
			EXPECT_EQ(outType.n, inType.n) << id;
		}

		/* locations number by id on both sides, so equal ids meet at equal locations */
		const auto fragmentLayout = assignLayout(fragmentEntries, fragment.types, {});
		ASSERT_EQ(fragmentLayout.inputLocations.size(), 2uz);
		EXPECT_EQ(fragmentLayout.inputLocations[0], 0u);
		EXPECT_EQ(fragmentLayout.inputLocations[1], 1u);

		const auto vertexLayout = assignLayout(vertexEntries, vertex.types, {});
		const DenseMap<String, u32> offsets {
			{ "vertex-position", 0u }, { "vertex-color", 8u }, { "vertex-uv", 12u }, { "vertex-layer", 24u }
		};
		const auto vertexLowered = lowerToIntermediate(vertex, vertexLayout, Dialect::eVulkanGlsl460, offsets);
		ASSERT_TRUE(vertexLowered.has_value()) << vertexLowered.error().front().message;
		const auto fragmentLowered = lowerToIntermediate(fragment, fragmentLayout, Dialect::eVulkanGlsl460);
		ASSERT_TRUE(fragmentLowered.has_value()) << fragmentLowered.error().front().message;

		const auto& vertexText = vertexLowered->lang.text.front().data;
		const auto& fragmentText = fragmentLowered->lang.text.front().data;
		EXPECT_TRUE(vertexText.contains("layout(location = 0) out vec4")) << vertexText;
		EXPECT_TRUE(vertexText.contains("layout(location = 1) out vec3")) << vertexText;
		EXPECT_TRUE(fragmentText.contains("layout(location = 0) in vec4")) << fragmentText;
		EXPECT_TRUE(fragmentText.contains("layout(location = 1) in vec3")) << fragmentText;
	}

	TEST(UiShaderTest, VertexFormatsReachLoweredAnnotations) {
		const auto vertex = shaderAssetModule("ui/ui.vs.asl", EntryKind::eVertex);
		const auto layout = assignLayout(computeInterface(vertex), vertex.types, {});
		const DenseMap<String, u32> offsets {
			{ "vertex-position", 0u }, { "vertex-color", 8u }, { "vertex-uv", 12u }, { "vertex-layer", 24u }
		};
		const DenseMap<String, s32> formats { { "vertex-color", 37 }, { "vertex-layer", 9 } };

		const auto lowered = lowerToIntermediate(vertex, layout, Dialect::eVulkanGlsl460, offsets, formats);
		ASSERT_TRUE(lowered.has_value()) << lowered.error().front().message;

		const auto formatOf = [&](StringView id_) -> Opt<s32> {
			for (const auto& var : lowered->rep.globalScope.inbound) {
				const SymbolIdAnnotation* id = nullptr;
				const VkBindLocationAnnotation* location = nullptr;
				for (auto* cur = var->annotation.get(); cur != nullptr; cur = cur->next.get()) {
					if (cur->type == AnnotationType::eSymbolId) {
						id = static_cast<const SymbolIdAnnotation*>(cur);
					} else if (cur->type == AnnotationType::eVkBindLocation) {
						location = static_cast<const VkBindLocationAnnotation*>(cur);
					}
				}
				if (id != nullptr && location != nullptr && id->symbolId == id_) {
					return location->vkFormat;
				}
			}
			return None;
		};

		EXPECT_EQ(formatOf("vertex-color"), 37);
		EXPECT_EQ(formatOf("vertex-layer"), 9);
		EXPECT_EQ(formatOf("vertex-position"), -1);
	}
}
