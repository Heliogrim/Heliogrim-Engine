#include "pch.h"

#include "IrSpirvGate.hpp"

#include <Engine.Accel.Compile/Pass/VkVertexFormat.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/EffectSpecification.hpp>
#include <Engine.Accel.Compile/Stage/StageIr.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Common/Make.hpp>

using namespace hg::engine::accel::lang::ir;
using namespace hg::engine::accel::lang;
using namespace hg::engine::accel;
using namespace hg;

namespace AccelCompile {
	namespace {
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

		/* a single stage is its own pipeline; a fresh layout cache unless one is given */
		[[nodiscard]] std::expected<smr<StageDerivat>, Diagnostics> composeStageIr(
			ref<const Intermediate> stage_,
			ref<const EffectProfile> profile_,
			ref<const EffectSpecification> spec_,
			const StageFlagBits flags_,
			ref<VariantCache> cache_,
			smr<const ResolvedPipeline>* pipeline_ = nullptr
		) {
			PipelineLayoutCache layouts {};
			const std::array<const Intermediate*, 1> stages { &stage_ };
			const std::array<StageFlagBits, 1> flags { flags_ };
			auto composed = composePipelineIr(stages, flags, profile_, spec_, cache_, layouts, pipeline_);
			if (not composed.has_value()) {
				return std::unexpected(std::move(composed).error());
			}
			return composed->front();
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

	TEST(StageIrTest, LibraryBuiltinStageValidityIsCheckedPerComposedStage) {
		constexpr auto library = "fn Lib() { position = f32vec4(0.0); }\n";
		constexpr auto main = "@output (mode = forward, id = \"color\") f32vec4 color;\nfn main() { color = f32vec4(0.0); }\n";

		VariantCache cache {};
		const auto fragment = composeStageIr(
			intermediateOf(main, EntryKind::eFragment),
			profileOf(library),
			TestSpecification { {} },
			StageFlagBits::eFragment,
			cache
		);
		ASSERT_FALSE(fragment.has_value());
		EXPECT_TRUE(fragment.error().front().message.contains("is not available in fragment stages"));

		const auto vertex = composeStageIr(
			intermediateOf(main, EntryKind::eVertex),
			profileOf(library),
			TestSpecification { {} },
			StageFlagBits::eVertex,
			cache
		);
		EXPECT_TRUE(vertex.has_value()) << (vertex.has_value() ? "" : vertex.error().front().message);
	}

	TEST(StageIrTest, ModuleWarningsNeverFailCompose) {
		auto stage = intermediateOf("@output (mode = forward, id = \"color\") f32vec4 color;\nfn main() { color = f32vec4(0.0); }\n", EntryKind::eFragment);
		auto module = *stage.ir;
		module.warnings.emplace_back(Diagnostic { .severity = Severity::eWarning, .message = "synthetic warning" });
		stage.ir = make_smr<const Module>(new Module { std::move(module) });

		VariantCache cache {};
		const auto derivat = composeStageIr(stage, EffectProfile {}, TestSpecification { {} }, StageFlagBits::eFragment, cache);
		EXPECT_TRUE(derivat.has_value());
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

		const auto lowText = composeStageIr(toggledStage, EffectProfile {}, spec, StageFlagBits::eFragment, cache);
		const auto highText = composeStageIr(toggledStage, high, spec, StageFlagBits::eFragment, cache);
		ASSERT_TRUE(lowText.has_value() && highText.has_value());
		EXPECT_NE(textOf(**lowText), textOf(**highText));

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

		smr<const ResolvedPipeline> pipeline {};
		const auto derivat = composeStageIr(stage, profile, spec, StageFlagBits::eFragment, cache, &pipeline);
		ASSERT_TRUE(derivat.has_value());
		ASSERT_EQ(pipeline->layouts.front().push.size(), 1uLL);

		BindLayout layout {};
		const std::array<smr<StageDerivat>, 1> derivats { *derivat };
		EXPECT_TRUE(appendBindLayout(*pipeline, derivats, layout).empty());

		ASSERT_EQ(layout.constants.size(), 1uLL);
		EXPECT_EQ(layout.constants.front().size, pipeline->layouts.front().push.front().size);
		EXPECT_EQ(layout.constants.front().size, 4uL);
		EXPECT_EQ(layout.constants.front().offset, 0uL);
		EXPECT_TRUE(layout.constants.front().stages & StageFlagBits::eFragment);

		/* the uniform lands in set 0 at its layout position */
		ASSERT_EQ(layout.groups.size(), 1uLL);
		ASSERT_EQ(layout.groups.front().elements.size(), 1uLL);
		EXPECT_EQ(layout.groups.front().elements.front().symbol->symbolName, "camera");
		EXPECT_EQ(layout.groups.front().elements.front().type, BindType::eUniformBuffer);
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

		struct Composed {
			Vector<smr<StageDerivat>> derivats;
			smr<const ResolvedPipeline> pipeline;
		};

		/* vertex ids, then fragment ids, as one pipeline */
		[[nodiscard]] std::expected<Composed, Diagnostics> composeUsing(
			StringView vertexIds_,
			StringView fragmentIds_,
			ref<VariantCache> cache_,
			ref<PipelineLayoutCache> layouts_
		) {
			const auto vertex = intermediateOf(fragmentUsing(vertexIds_), EntryKind::eVertex);
			const auto fragment = intermediateOf(fragmentUsing(fragmentIds_), EntryKind::eFragment);
			const std::array<const Intermediate*, 2> stages { &vertex, &fragment };
			const std::array<StageFlagBits, 2> flags { StageFlagBits::eVertex, StageFlagBits::eFragment };

			Composed result {};
			auto composed = composePipelineIr(
				stages, flags, EffectProfile {}, TestSpecification { {} }, cache_, layouts_, &result.pipeline
			);
			if (not composed.has_value()) {
				return std::unexpected(std::move(composed).error());
			}
			result.derivats = std::move(composed).value();
			return result;
		}
	}

	TEST(StageIrTest, DifferentIdsInStagesGetDistinctSlots) {
		VariantCache cache {};
		PipelineLayoutCache layouts {};
		const auto composed = composeUsing("a", "b", cache, layouts);
		ASSERT_TRUE(composed.has_value()) << composed.error().front().message;

		BindLayout layout {};
		EXPECT_TRUE(appendBindLayout(*composed->pipeline, composed->derivats, layout).empty());

		/* sorted by id over the pipeline: a = binding 0, b = binding 1 */
		ASSERT_EQ(layout.groups.size(), 1uLL);
		ASSERT_EQ(layout.groups.front().elements.size(), 2uLL);
		EXPECT_EQ(layout.groups.front().elements[0].symbol->symbolName, "a");
		EXPECT_EQ(layout.groups.front().elements[1].symbol->symbolName, "b");
	}

	TEST(StageIrTest, StageWithoutPrefixOfPipelineBindingsLaysOut) {
		VariantCache cache {};
		PipelineLayoutCache layouts {};

		/* vertex {b} is not a prefix of the union {a, b} */
		const auto composed = composeUsing("b", "ab", cache, layouts);
		ASSERT_TRUE(composed.has_value()) << composed.error().front().message;

		BindLayout layout {};
		const auto diagnostics = appendBindLayout(*composed->pipeline, composed->derivats, layout);
		ASSERT_TRUE(diagnostics.empty()) << diagnostics.front().message;
		ASSERT_EQ(layout.groups.size(), 1uLL);
		ASSERT_EQ(layout.groups.front().elements.size(), 2uLL);
		EXPECT_EQ(layout.groups.front().elements[0].symbol->symbolName, "a");
		EXPECT_EQ(layout.groups.front().elements[1].symbol->symbolName, "b");
	}

	TEST(StageIrTest, VertexAndFragmentWithInterleavedBindingsLayOut) {
		VariantCache cache {};
		PipelineLayoutCache layouts {};

		/* camera = c, model = m, lights = l, textures = t: union c, l, m, t; the vertex stage skips l */
		const auto composed = composeUsing("cm", "clt", cache, layouts);
		ASSERT_TRUE(composed.has_value()) << composed.error().front().message;

		BindLayout layout {};
		const auto diagnostics = appendBindLayout(*composed->pipeline, composed->derivats, layout);
		ASSERT_TRUE(diagnostics.empty()) << diagnostics.front().message;
		ASSERT_EQ(layout.groups.size(), 1uLL);
		ASSERT_EQ(layout.groups.front().elements.size(), 4uLL);
		const std::array<StringView, 4> expected { "c", "l", "m", "t" };
		for (size_t idx = 0uLL; idx < expected.size(); ++idx) {
			EXPECT_EQ(layout.groups.front().elements[idx].symbol->symbolName, expected[idx]);
		}
	}

	TEST(StageIrTest, MismatchedVaryingFailsCompose) {
		const auto vertex = intermediateOf(
			"@output (mode = forward, id = \"uv\") f32vec3 uv;\nfn main() { uv = f32vec3(0.0); position = f32vec4(0.0); }\n",
			EntryKind::eVertex
		);
		const auto fragment = intermediateOf(
			"@input (mode = forward, id = \"uv\") f32vec2 uv;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() { color = f32vec4(uv, 0.0, 1.0); }\n",
			EntryKind::eFragment
		);

		VariantCache cache {};
		PipelineLayoutCache layouts {};
		const std::array<const Intermediate*, 2> stages { &vertex, &fragment };
		const std::array<StageFlagBits, 2> flags { StageFlagBits::eVertex, StageFlagBits::eFragment };
		const auto composed = composePipelineIr(stages, flags, EffectProfile {}, TestSpecification { {} }, cache, layouts);
		ASSERT_FALSE(composed.has_value());
		const auto& message = composed.error().front().message;
		EXPECT_TRUE(message.contains("'uv'") && message.contains("f32vec2") && message.contains("f32vec3")) << message;

		/* same stages with a legacy text stage between them (follows = false): no chain check across the gap */
		PipelineLayoutCache gapLayouts {};
		const std::array<u8, 2> follows { 1u, 0u };
		const auto gap = composePipelineIr(
			stages, flags, EffectProfile {}, TestSpecification { {} }, cache, gapLayouts, nullptr, true, follows
		);
		EXPECT_TRUE(gap.has_value()) << (gap.has_value() ? "" : gap.error().front().message);
	}

	TEST(StageIrTest, ConsumerInputTakesProducerOutputLocation) {
		const auto vertex = intermediateOf(
			"@output (mode = forward, id = \"a\") f32vec4 a;\n@output (mode = forward, id = \"b\") f32vec4 b;\n"
			"@output (mode = forward, id = \"c\") f32vec4 c;\n"
			"fn main() { a = f32vec4(0.0); b = f32vec4(1.0); c = f32vec4(2.0); position = f32vec4(0.0); }\n",
			EntryKind::eVertex
		);
		const auto fragment = intermediateOf(
			"@input (mode = forward, id = \"c\") f32vec4 c;\n@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() { color = c; }\n",
			EntryKind::eFragment
		);

		VariantCache cache {};
		PipelineLayoutCache layouts {};
		const std::array<const Intermediate*, 2> stages { &vertex, &fragment };
		const std::array<StageFlagBits, 2> flags { StageFlagBits::eVertex, StageFlagBits::eFragment };
		const auto composed = composePipelineIr(stages, flags, EffectProfile {}, TestSpecification { {} }, cache, layouts);
		ASSERT_TRUE(composed.has_value()) << composed.error().front().message;
		const auto vertexText = textOf(*(*composed)[0]);
		const auto fragmentText = textOf(*(*composed)[1]);
		EXPECT_TRUE(vertexText.contains("layout(location = 2) out")) << vertexText;
		EXPECT_TRUE(fragmentText.contains("layout(location = 2) in")) << fragmentText;
		EXPECT_FALSE(fragmentText.contains("layout(location = 0) in")) << fragmentText;

		/* same stages, not directly following: the cached layout must not be reused */
		const std::array<u8, 2> apart { 1u, 0u };
		const auto split = composePipelineIr(stages, flags, EffectProfile {}, TestSpecification { {} }, cache, layouts, nullptr, true, apart);
		ASSERT_TRUE(split.has_value()) << split.error().front().message;
		const auto splitText = textOf(*(*split)[1]);
		EXPECT_TRUE(splitText.contains("layout(location = 0) in")) << splitText;
	}

	TEST(StageIrTest, EmptyStageListIsAnError) {
		VariantCache cache {};
		PipelineLayoutCache layouts {};
		const auto composed = composePipelineIr({}, {}, EffectProfile {}, TestSpecification { {} }, cache, layouts);
		ASSERT_FALSE(composed.has_value());
		EXPECT_EQ(composed.error().front().severity, Severity::eError);
	}

	TEST(StageIrTest, SharedCameraAcrossStagesComposes) {
		VariantCache cache {};
		PipelineLayoutCache layouts {};

		/* alone, b would be binding 0 in the fragment stage; the pipeline gives it one slot in both */
		const auto composed = composeUsing("ab", "b", cache, layouts);
		ASSERT_TRUE(composed.has_value()) << composed.error().front().message;

		const auto& vertexSlots = composed->pipeline->layouts[0].slots;
		const auto& fragmentSlots = composed->pipeline->layouts[1].slots;
		ASSERT_EQ(fragmentSlots.size(), 1uLL);
		const auto shared = std::ranges::find_if(vertexSlots, [](const auto& s_) { return s_.id == "b"; });
		ASSERT_NE(shared, vertexSlots.end());
		EXPECT_EQ(shared->binding, fragmentSlots.front().binding);
		EXPECT_EQ(shared->set, fragmentSlots.front().set);

		BindLayout layout {};
		EXPECT_TRUE(appendBindLayout(*composed->pipeline, composed->derivats, layout).empty());
		ASSERT_EQ(layout.groups.size(), 1uLL);
		EXPECT_EQ(layout.groups.front().elements.size(), 2uLL);
	}

	TEST(StageIrTest, SecondPipelineComposeHitsLayoutCache) {
		VariantCache cache {};
		PipelineLayoutCache layouts {};

		const auto first = composeUsing("ab", "b", cache, layouts);
		const auto second = composeUsing("ab", "b", cache, layouts);
		ASSERT_TRUE(first.has_value() && second.has_value());
		EXPECT_EQ(first->pipeline.get(), second->pipeline.get());

		/* every composed derivat owns its own intermediate */
		EXPECT_NE(first->derivats[1]->getIntermediate().get(), second->derivats[1]->getIntermediate().get());
		EXPECT_EQ(textOf(*first->derivats[1]), textOf(*second->derivats[1]));

		/* other stage content is another pipeline */
		const auto other = composeUsing("ab", "a", cache, layouts);
		ASSERT_TRUE(other.has_value());
		EXPECT_NE(first->pipeline.get(), other->pipeline.get());
	}

	TEST(StageIrTest, PipelineHitResolvesNoStage) {
		VariantCache cache {};
		PipelineLayoutCache layouts {};
		const auto first = composeUsing("ab", "b", cache, layouts);
		ASSERT_TRUE(first.has_value());

		VariantCache untouched {};
		const auto second = composeUsing("ab", "b", untouched, layouts);
		ASSERT_TRUE(second.has_value());
		EXPECT_EQ(first->pipeline.get(), second->pipeline.get());
		for (const auto& stage : second->pipeline->stages) {
			EXPECT_EQ(untouched.find(stage->key), nullptr);
		}
	}

	TEST(StageIrTest, DistinctPushIdsPublishTheirOffset) {
		const auto pushStage = [](StringView id_, EntryKind entry_) {
			const String id { id_ };
			return intermediateOf(
				"@input (mode = bind, id = \"" + id + "\", class = push) block " + id + " { u32 v; };\n"
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn main() { color = f32vec4(f32(" + id + ".v)); }\n",
				entry_
			);
		};
		const auto vertex = pushStage("a", EntryKind::eVertex);
		const auto fragment = pushStage("b", EntryKind::eFragment);
		const std::array<const Intermediate*, 2> stages { &vertex, &fragment };
		const std::array<StageFlagBits, 2> flags { StageFlagBits::eVertex, StageFlagBits::eFragment };

		VariantCache cache {};
		PipelineLayoutCache layouts {};
		smr<const ResolvedPipeline> pipeline {};
		auto derivats = composePipelineIr(stages, flags, EffectProfile {}, TestSpecification { {} }, cache, layouts, &pipeline);
		ASSERT_TRUE(derivats.has_value()) << derivats.error().front().message;

		ASSERT_EQ(pipeline->layouts[1].push.size(), 1uLL);
		EXPECT_EQ(pipeline->layouts[1].push.front().offset, 4uL);
		EXPECT_NE(textOf(*(*derivats)[1]).find("layout(offset = 4)"), String::npos);
		EXPECT_EQ(textOf(*(*derivats)[0]).find("layout(offset"), String::npos);

		BindLayout layout {};
		EXPECT_TRUE(appendBindLayout(*pipeline, *derivats, layout).empty());
		ASSERT_EQ(layout.constants.size(), 2uLL);
		EXPECT_EQ(layout.constants[1].offset, 4uL);
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
		PipelineLayoutCache layouts {};

		/* the export stage is the last one: the vertex stage keeps its varying although the target names a fragment output */
		const std::array<const Intermediate*, 2> stages { &vertex, &fragment };
		const std::array<StageFlagBits, 2> flags { StageFlagBits::eVertex, StageFlagBits::eFragment };
		const auto both = composePipelineIr(stages, flags, profile, spec, cache, layouts);
		ASSERT_TRUE(both.has_value()) << both.error().front().message;
		EXPECT_TRUE(textOf(*(*both)[0]).contains("uv")) << textOf(*(*both)[0]);
		EXPECT_FALSE(textOf(*(*both)[1]).contains("other")) << textOf(*(*both)[1]);

		/* alone, the vertex stage is the export stage and would fail */
		EXPECT_FALSE(composeStageIr(vertex, EffectProfile {}, spec, StageFlagBits::eVertex, cache).has_value());

		/* the same fragment ahead of another stage keeps every output */
		const std::array<const Intermediate*, 2> inner { &fragment, &fragment };
		const std::array<StageFlagBits, 2> innerFlags { StageFlagBits::eFragment, StageFlagBits::eFragment };
		const auto kept = composePipelineIr(inner, innerFlags, profile, spec, cache, layouts);
		ASSERT_TRUE(kept.has_value()) << kept.error().front().message;
		EXPECT_TRUE(textOf(*(*kept)[0]).contains("other")) << textOf(*(*kept)[0]);

		/* a legacy text stage after the ir stages is the export stage: nothing here is sliced */
		const auto notLast = composePipelineIr(
			stages, flags, profile, spec, cache, layouts, nullptr, false
		);
		ASSERT_TRUE(notLast.has_value()) << notLast.error().front().message;
		EXPECT_TRUE(textOf(*(*notLast)[1]).contains("other")) << textOf(*(*notLast)[1]);
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

		const auto resolved = composeStageIr(vertex, EffectProfile {}, TestSpecification { {} }, StageFlagBits::eVertex, cache);
		ASSERT_TRUE(resolved.has_value()) << resolved.error().front().message;

		const auto formatOf = [&](StringView id_) {
			for (const auto& var : (*resolved)->getIntermediate()->rep.globalScope.inbound) {
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
