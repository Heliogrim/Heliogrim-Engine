#include "pch.h"

#include <Engine.Accel.Compile/Pass/VkVertexFormat.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/EffectSpecification.hpp>
#include <Engine.Accel.Compile/Stage/StageIr.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Common/Make.hpp>

using namespace hg;
using namespace hg::engine::accel;
using namespace hg::engine::accel::lang;
using namespace hg::engine::accel::lang::ir;

namespace AccelCompile {
	namespace {
		class TestSpecification final :
			public EffectSpecification {
		public:
			explicit TestSpecification(const Vector<String>& targets_) {
				for (const auto& target : targets_) {
					auto symbol = make_smr<Symbol>(new Symbol {});
					symbol->symbolName = target;
					_targets.emplace_back(std::move(symbol).into<const Symbol>());
				}
			}

			[[nodiscard]] ref<const Vector<smr<const Symbol>>> targetSymbols() const noexcept override {
				return _targets;
			}

			[[nodiscard]] ComputePassSpecification getComputeSpec() const override {
				return {};
			}

			[[nodiscard]] GraphicsPassSpecification getGraphicsSpec() const override {
				return {};
			}

			[[nodiscard]] MeshPassSpecification getMeshSpec() const override {
				return {};
			}

			[[nodiscard]] RaytracingPassSpecification getRaytracingSpec() const override {
				return {};
			}

		private:
			Vector<smr<const Symbol>> _targets;
		};

		[[nodiscard]] Intermediate intermediateOf(StringView source_, EntryKind entry_) {
			auto parsed = parseModule(
				"M",
				source_,
				entry_,
				ModuleResolver { [](StringView) -> Opt<String> { return None; } }
			);
			EXPECT_TRUE(parsed.has_value()) << (parsed.has_value() || parsed.error().empty() ? "" : parsed.error().front().message);

			Intermediate result {};
			if (parsed.has_value()) {
				result.ir = make_smr<const Module>(new Module { std::move(parsed).value() });
			}
			return result;
		}

		constexpr auto profileSource =
			"@input (mode = bind, id = \"camera\", class = uniform) block camera { f32vec4 tint; };\n"
			"@input (mode = bind, id = \"block\", class = push) block push { u32 cascade; };\n"
			"fn Tint() -> f32vec4 { return camera.tint * f32(push.cascade); }\n";

		constexpr auto stageSource =
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"@output (mode = forward, id = \"other\") f32vec4 other;\n"
			"fn Tint() -> f32vec4;\n"
			"fn main() {\n"
			"\tcolor = Tint();\n"
			"\tother = f32vec4(0.0, 0.0, 0.0, 1.0);\n"
			"}\n";

		[[nodiscard]] EffectProfile profileOf(StringView source_ = profileSource) {
			EffectProfile profile {};
			profile._name = "Test";
			profile._modules.emplace_back(intermediateOf(source_, EntryKind::eFragment));
			return profile;
		}

		[[nodiscard]] String textOf(ref<const StageDerivat> derivat_) {
			const auto im = derivat_.getIntermediate();
			return im->lang.text.empty() ? String {} : im->lang.text.front().data;
		}
	}

	TEST(StageIrTest, UsesProfileModuleBodiesInsteadOfTextConcat) {
		VariantCache cache {};
		const auto stage = intermediateOf(stageSource, EntryKind::eFragment);

		auto derivat = composeStageIr(stage, profileOf(), TestSpecification { {} }, StageFlagBits::eFragment, cache);
		ASSERT_TRUE(derivat.has_value()) << derivat.error().front().message;

		const auto text = textOf(**derivat);
		EXPECT_TRUE(text.starts_with("#version 460\n")) << text;
		EXPECT_TRUE(text.contains("camera.tint")) << text;
		EXPECT_TRUE(text.contains("Tint()")) << text;
		EXPECT_FALSE(text.contains("HG_SCTX")) << text;
		EXPECT_EQ((*derivat)->getFlagBits(), StageFlagBits::eFragment);
	}

	TEST(StageIrTest, TargetSymbolsSliceOutputs) {
		VariantCache cache {};
		const auto stage = intermediateOf(stageSource, EntryKind::eFragment);

		auto all = composeStageIr(stage, profileOf(), TestSpecification { {} }, StageFlagBits::eFragment, cache);
		auto some = composeStageIr(
			stage,
			profileOf(),
			TestSpecification { { "color" } },
			StageFlagBits::eFragment,
			cache
		);
		ASSERT_TRUE(all.has_value());
		ASSERT_TRUE(some.has_value()) << some.error().front().message;

		EXPECT_TRUE(textOf(**all).contains("other")) << textOf(**all);
		EXPECT_TRUE(textOf(**some).contains("color")) << textOf(**some);
		EXPECT_FALSE(textOf(**some).contains("other")) << textOf(**some);

		const auto unknown = composeStageIr(
			stage,
			profileOf(),
			TestSpecification { { "missing" } },
			StageFlagBits::eFragment,
			cache
		);
		ASSERT_FALSE(unknown.has_value());
		EXPECT_TRUE(unknown.error().front().message.contains("missing"));
	}

	TEST(StageIrTest, SecondComposeHitsCache) {
		VariantCache cache {};
		const auto stage = intermediateOf(stageSource, EntryKind::eFragment);
		const auto profile = profileOf();
		const TestSpecification spec { {} };

		const auto first = resolveStageIr(stage, profile, spec, StageFlagBits::eFragment, cache);
		const auto second = resolveStageIr(stage, profile, spec, StageFlagBits::eFragment, cache);
		ASSERT_TRUE(first.has_value());
		ASSERT_TRUE(second.has_value());
		EXPECT_EQ(first->get(), second->get());

		/* equal content in other objects hits as well, another toggle value is another variant */
		const auto sameContent = resolveStageIr(
			intermediateOf(stageSource, EntryKind::eFragment),
			profileOf(),
			spec,
			StageFlagBits::eFragment,
			cache
		);
		ASSERT_TRUE(sameContent.has_value());
		EXPECT_EQ(first->get(), sameContent->get());

		const auto toggledStage = intermediateOf(
			"toggle Quality : { Low, High } = Low;\n"
			"@output (mode = forward, id = \"color\") f32 color;\n"
			"@when (Quality == High) fn Pick() -> f32 { return 1.0; }\n"
			"@when (Quality == Low) fn Pick() -> f32 { return 0.5; }\n"
			"fn main() { color = Pick(); }\n",
			EntryKind::eFragment
		);
		auto high = EffectProfile {};
		high._toggles.emplace_back("Quality", "High");

		const auto lowVariant = resolveStageIr(toggledStage, EffectProfile {}, spec, StageFlagBits::eFragment, cache);
		const auto highVariant = resolveStageIr(toggledStage, high, spec, StageFlagBits::eFragment, cache);
		ASSERT_TRUE(lowVariant.has_value() && highVariant.has_value());
		EXPECT_NE(lowVariant->get(), highVariant->get());
		EXPECT_NE(lowVariant->get()->lowered.lang.text.front().data, highVariant->get()->lowered.lang.text.front().data);

		const auto highAgain = resolveStageIr(toggledStage, high, spec, StageFlagBits::eFragment, cache);
		ASSERT_TRUE(highAgain.has_value());
		EXPECT_EQ(highVariant->get(), highAgain->get());

		/* a toggle the module does not declare is an error, not a cached variant */
		auto unknown = profileOf();
		unknown._toggles.emplace_back("quality", "high");
		EXPECT_FALSE(resolveStageIr(stage, unknown, spec, StageFlagBits::eFragment, cache).has_value());

		/* every composed derivat owns its own intermediate */
		const auto a = composeStageIr(stage, profile, spec, StageFlagBits::eFragment, cache);
		const auto b = composeStageIr(stage, profile, spec, StageFlagBits::eFragment, cache);
		ASSERT_TRUE(a.has_value() && b.has_value());
		EXPECT_NE((*a)->getIntermediate().get(), (*b)->getIntermediate().get());
		EXPECT_EQ(textOf(**a), textOf(**b));
	}

	TEST(StageIrTest, ContractFailureReturnsDiagnostics) {
		VariantCache cache {};
		const auto stage = intermediateOf(
			"@input (mode = bind, id = \"noclass\") block noclass { f32 a; };\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() { color = f32vec4(noclass.a, 0.0, 0.0, 1.0); }\n",
			EntryKind::eFragment
		);

		auto derivat = composeStageIr(stage, EffectProfile {}, TestSpecification { {} }, StageFlagBits::eFragment, cache);
		ASSERT_FALSE(derivat.has_value());
		ASSERT_FALSE(derivat.error().empty());
		EXPECT_TRUE(derivat.error().front().message.contains("noclass")) << derivat.error().front().message;
		EXPECT_EQ(derivat.error().front().severity, Severity::eError);
	}

	TEST(StageIrTest, PushConstantSizeComesFromLayout) {
		VariantCache cache {};
		const auto stage = intermediateOf(stageSource, EntryKind::eFragment);
		const auto profile = profileOf();
		const TestSpecification spec { {} };

		const auto resolved = resolveStageIr(stage, profile, spec, StageFlagBits::eFragment, cache);
		ASSERT_TRUE(resolved.has_value());
		ASSERT_EQ((*resolved)->layout.push.size(), 1uLL);

		const auto derivat = composeStageIr(stage, profile, spec, StageFlagBits::eFragment, cache);
		ASSERT_TRUE(derivat.has_value());

		BindLayout layout {};
		EXPECT_TRUE(appendBindLayout(**resolved, **derivat, layout).empty());

		ASSERT_EQ(layout.constants.size(), 1uLL);
		EXPECT_EQ(layout.constants.front().size, (*resolved)->layout.push.front().size);
		EXPECT_EQ(layout.constants.front().size, 4uL);
		EXPECT_EQ(layout.constants.front().offset, 0uL);
		EXPECT_TRUE(layout.constants.front().stages & StageFlagBits::eFragment);

		/* the uniform lands in set 0 at its layout position */
		ASSERT_EQ(layout.groups.size(), 1uLL);
		ASSERT_EQ(layout.groups.front().elements.size(), 1uLL);
		EXPECT_EQ(layout.groups.front().elements.front().symbol->symbolName, "camera");
		EXPECT_EQ(layout.groups.front().elements.front().type, BindType::eUniformBuffer);

		/* a second stage of the same layout adds its flag and no duplicates */
		EXPECT_TRUE(appendBindLayout(**resolved, **derivat, layout).empty());
		EXPECT_EQ(layout.constants.size(), 1uLL);
		EXPECT_EQ(layout.groups.front().elements.size(), 1uLL);
	}

	namespace {
		[[nodiscard]] String fragmentUsing(StringView ids_) {
			String source {};
			String sum { "f32vec4(0.0)" };
			for (const auto id : ids_) {
				const String name(1uLL, id);
				source += "@input (mode = bind, id = \"" + name + "\", class = uniform) block " + name +
					" { f32vec4 v; };\n";
				sum += " + " + name + ".v";
			}
			return source + "@output (mode = forward, id = \"color\") f32vec4 color;\nfn main() { color = " + sum + "; }\n";
		}

		struct Laid {
			smr<const Resolved> resolved;
			smr<StageDerivat> derivat;
		};

		[[nodiscard]] Laid layoutOf(StringView ids_, ref<VariantCache> cache_, StageFlagBits flags_) {
			const auto stage = intermediateOf(fragmentUsing(ids_), EntryKind::eFragment);
			const TestSpecification spec { {} };
			auto resolved = resolveStageIr(stage, EffectProfile {}, spec, flags_, cache_);
			auto derivat = composeStageIr(stage, EffectProfile {}, spec, flags_, cache_);
			EXPECT_TRUE(resolved.has_value() && derivat.has_value());
			return { std::move(resolved).value(), std::move(derivat).value() };
		}
	}

	TEST(StageIrTest, LayoutGuardRejectsDifferentIdsOnOneSlot) {
		VariantCache cache {};
		const auto first = layoutOf("a", cache, StageFlagBits::eVertex);
		const auto second = layoutOf("b", cache, StageFlagBits::eFragment);

		BindLayout layout {};
		EXPECT_TRUE(appendBindLayout(*first.resolved, *first.derivat, layout).empty());

		const auto clash = appendBindLayout(*second.resolved, *second.derivat, layout);
		ASSERT_FALSE(clash.empty());
		EXPECT_TRUE(clash.front().message.contains("'b'")) << clash.front().message;
		EXPECT_EQ(layout.groups.front().elements.size(), 1uLL);
	}

	TEST(StageIrTest, LayoutGuardRejectsSharedIdAtDifferentSlots) {
		VariantCache cache {};
		/* sorted by id: a = binding 0, b = binding 1; alone, b = binding 0 */
		const auto first = layoutOf("ab", cache, StageFlagBits::eVertex);
		const auto second = layoutOf("b", cache, StageFlagBits::eFragment);

		BindLayout layout {};
		EXPECT_TRUE(appendBindLayout(*first.resolved, *first.derivat, layout).empty());

		const auto clash = appendBindLayout(*second.resolved, *second.derivat, layout);
		ASSERT_FALSE(clash.empty());
		EXPECT_TRUE(clash.front().message.contains("'b'")) << clash.front().message;
	}

	TEST(StageIrTest, LayoutGuardAcceptsSameSlotsAndSupersets) {
		VariantCache cache {};
		const auto first = layoutOf("a", cache, StageFlagBits::eVertex);
		const auto second = layoutOf("ab", cache, StageFlagBits::eFragment);

		BindLayout layout {};
		EXPECT_TRUE(appendBindLayout(*first.resolved, *first.derivat, layout).empty());
		EXPECT_TRUE(appendBindLayout(*second.resolved, *second.derivat, layout).empty());
		ASSERT_EQ(layout.groups.size(), 1uLL);
		EXPECT_EQ(layout.groups.front().elements.size(), 2uLL);
	}

	TEST(StageIrTest, TargetSymbolsOnlySliceTheExportStage) {
		VariantCache cache {};
		const auto vertex = intermediateOf(
			"@output (mode = forward, id = \"uv\") f32vec2 uv;\n"
			"fn main() { uv = f32vec2(0.0, 1.0); }\n",
			EntryKind::eVertex
		);
		const auto fragment = intermediateOf(stageSource, EntryKind::eFragment);
		const TestSpecification spec { { "color" } };
		const auto profile = profileOf();

		/* the vertex stage keeps its varying although the target names a fragment output */
		const auto kept = composeStageIr(vertex, EffectProfile {}, spec, StageFlagBits::eVertex, cache, false);
		ASSERT_TRUE(kept.has_value()) << kept.error().front().message;
		EXPECT_TRUE(textOf(**kept).contains("uv")) << textOf(**kept);

		/* as the export stage it would fail */
		EXPECT_FALSE(composeStageIr(vertex, EffectProfile {}, spec, StageFlagBits::eVertex, cache, true).has_value());

		/* the export stage slices to the target */
		const auto exported = composeStageIr(fragment, profile, spec, StageFlagBits::eFragment, cache, true);
		ASSERT_TRUE(exported.has_value());
		EXPECT_FALSE(textOf(**exported).contains("other")) << textOf(**exported);

		/* the same fragment as a non-export stage keeps every output */
		const auto inner = composeStageIr(fragment, profile, spec, StageFlagBits::eFragment, cache, false);
		ASSERT_TRUE(inner.has_value());
		EXPECT_TRUE(textOf(**inner).contains("other")) << textOf(**inner);
	}

	TEST(StageIrTest, VertexFormatSuppliedByStageWinsOverShaderType) {
		VariantCache cache {};
		auto vertex = intermediateOf(
			"@input (mode = forward, id = \"vertex-color\") f32vec4 color;\n"
			"@input (mode = forward, id = \"vertex-position\") f32vec3 local;\n"
			"@output (mode = forward, id = \"forwarded\") f32vec4 forwarded;\n"
			"fn main() { forwarded = color; position = f32vec4(local, 1.0); }\n",
			EntryKind::eVertex
		);

		const auto attribute = [&](StringView id_, s32 offset_, s32 format_) {
			auto var = make_uptr<Variable>();
			var->annotation = make_uptr<VkBindLocationAnnotation>(
				-1L,
				0L,
				offset_,
				make_uptr<SymbolIdAnnotation>(String { id_ }),
				format_
			);
			vertex.rep.globalScope.inbound.emplace_back(std::move(var));
		};
		attribute("vertex-color", 8L, static_cast<s32>(vk::Format::eR8G8B8A8Unorm));
		attribute("vertex-position", 0L, -1L);

		const auto resolved = resolveStageIr(
			vertex,
			EffectProfile {},
			TestSpecification { {} },
			StageFlagBits::eVertex,
			cache,
			false
		);
		ASSERT_TRUE(resolved.has_value()) << resolved.error().front().message;

		const auto formatOf = [&](StringView id_) {
			for (const auto& var : (*resolved)->lowered.rep.globalScope.inbound) {
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
					return vertexFormatOf(var->type.scalarType, location->vkFormat);
				}
			}
			return vk::Format::eUndefined;
		};

		EXPECT_EQ(formatOf("vertex-color"), vk::Format::eR8G8B8A8Unorm);
		EXPECT_EQ(formatOf("vertex-position"), vk::Format::eR32G32B32Sfloat);
	}
}
