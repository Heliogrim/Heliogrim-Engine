#include "pch.h"

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

		/* equal content in other objects hits as well, another profile toggle does not */
		const auto sameContent = resolveStageIr(
			intermediateOf(stageSource, EntryKind::eFragment),
			profileOf(),
			spec,
			StageFlagBits::eFragment,
			cache
		);
		ASSERT_TRUE(sameContent.has_value());
		EXPECT_EQ(first->get(), sameContent->get());

		auto toggled = profileOf();
		toggled._toggles.emplace_back("quality", "high");
		const auto other = resolveStageIr(stage, toggled, spec, StageFlagBits::eFragment, cache);
		/* the toggle is unknown to the module, which is an error and not a cached variant */
		EXPECT_FALSE(other.has_value());

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
		appendBindLayout(**resolved, **derivat, layout);

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
		appendBindLayout(**resolved, **derivat, layout);
		EXPECT_EQ(layout.constants.size(), 1uLL);
		EXPECT_EQ(layout.groups.front().elements.size(), 1uLL);
	}
}
