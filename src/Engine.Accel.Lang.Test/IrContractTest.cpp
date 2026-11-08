#include "pch.h"

#include <array>
#include <Engine.Accel.Lang/Ir/Contract.hpp>

using namespace hg;
using namespace hg::engine::accel::lang::ir;

namespace AccelLang {
	namespace {
		[[nodiscard]] InterfaceEntry bind(StringView id_, BindClass cls_) {
			return InterfaceEntry { String(id_), TypeRef { 0u }, BindMode::eBind, cls_, false, true };
		}

		[[nodiscard]] InterfaceEntry output(StringView id_, bool implicit_) {
			return InterfaceEntry { String(id_), TypeRef { 0u }, BindMode::eForward, BindClass::eNone, implicit_, false };
		}

		/* the shadow vertex + fragment interface, merged into one list for brevity */
		[[nodiscard]] Vector<InterfaceEntry> shader() {
			return {
				bind("block", BindClass::ePushConstant),
				bind("model", BindClass::eStorageBuffer),
				InterfaceEntry { "vertex-position", TypeRef { 0u }, BindMode::eForward, BindClass::eNone, false, true },
				bind("view", BindClass::eStorageBuffer),
				output("color", false),
				output("depth", true)
			};
		}
	}

	TEST(ContractTest, MissingMaterialBindingNamesId) {
		const StageContract stage { .provides = { "view", "block" }, .requires_ = {} };

		const auto diags = checkContract(shader(), stage, {});

		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(diags[0].severity, Severity::eError);
		EXPECT_TRUE(diags[0].message.contains("'model'")) << diags[0].message;
	}

	TEST(ContractTest, StageRequirementNotProvidedNamesId) {
		const StageContract stage { .provides = { "view", "block" }, .requires_ = { "depth", "normal" } };

		const auto diags = checkContract(shader(), stage, { "model" });

		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(diags[0].message.contains("'normal'")) << diags[0].message;
	}

	TEST(ContractTest, SatisfiedContractIsEmpty) {
		// implicit (depth) and non-implicit (color) outputs both satisfy requirements; forward inputs need no binding
		const StageContract stage { .provides = { "view" }, .requires_ = { "depth", "color" } };

		EXPECT_TRUE(checkContract(shader(), stage, { "model", "block" }).empty());
	}

	TEST(ContractTest, BindWithoutClassNamesId) {
		const auto diags = checkContract({ bind("albedo", BindClass::eNone) }, {}, { "albedo" });

		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(diags[0].message.contains("'albedo'")) << diags[0].message;
	}

	namespace {
		using Stage = std::pair<EntryKind, Vector<InterfaceEntry>>;

		[[nodiscard]] InterfaceEntry fwd(StringView id_, StringView type_, bool input_, bool arrayed_ = false) {
			return InterfaceEntry {
				String(id_), TypeRef { 0u }, BindMode::eForward, BindClass::eNone, false, input_, arrayed_, String(type_)
			};
		}
	}

	TEST(ContractTest, ChainMatchesByIdAndType) {
		const std::array<Stage, 2> ok {
			Stage { EntryKind::eVertex, { fwd("uv", "f32vec2", true), fwd("uv", "f32vec2", false), fwd("extra", "f32", false) } },
			Stage { EntryKind::eFragment, { fwd("uv", "f32vec2", true) } }
		};
		EXPECT_TRUE(checkStageChain(ok).empty());

		const std::array<Stage, 2> bad {
			Stage { EntryKind::eVertex, { fwd("uv", "f32vec3", false) } },
			Stage { EntryKind::eFragment, { fwd("uv", "f32vec2", true) } }
		};
		const auto diags = checkStageChain(bad);
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(diags[0].message.contains("f32vec2") && diags[0].message.contains("f32vec3")) << diags[0].message;
	}

	TEST(ContractTest, ChainMissingInputNamesStages) {
		const std::array<Stage, 2> stages {
			Stage { EntryKind::eGeometry, { fwd("a", "f32", false) } },
			Stage { EntryKind::eFragment, { fwd("world-pos", "f32vec4", true) } }
		};
		const auto diags = checkStageChain(stages);
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(diags[0].message, "fragment input 'world-pos' has no matching output in geometry stage");
	}

	TEST(ContractTest, ChainArrayedMatchesScalarOutput) {
		const std::array<Stage, 2> stages {
			Stage { EntryKind::eVertex, { fwd("world-pos", "f32vec4", false) } },
			Stage { EntryKind::eGeometry, { fwd("world-pos", "f32vec4", true, true) } }
		};
		EXPECT_TRUE(checkStageChain(stages).empty());
	}

	namespace {
		[[nodiscard]] InterfaceEntry patchFwd(StringView id_, StringView type_, bool input_) {
			auto entry = fwd(id_, type_, input_);
			entry.patch = true;
			return entry;
		}
	}

	TEST(ContractTest, ChainVertexTessGeomFragment) {
		const auto chain = [](bool withPatchOutput_) {
			Vector<InterfaceEntry> ctrl { fwd("pos", "f32vec4", true, true), fwd("cpos", "f32vec4", false, true) };
			if (withPatchOutput_) {
				ctrl.emplace_back(patchFwd("level", "f32", false));
			}
			return std::array<Stage, 5> {
				Stage { EntryKind::eVertex, { fwd("pos", "f32vec4", false) } },
				Stage { EntryKind::eTessCtrl, std::move(ctrl) },
				Stage {
					EntryKind::eTessEval,
					{ fwd("cpos", "f32vec4", true, true), patchFwd("level", "f32", true), fwd("e", "f32", false) }
				},
				Stage { EntryKind::eGeometry, { fwd("e", "f32", true, true), fwd("g", "f32", false) } },
				Stage { EntryKind::eFragment, { fwd("g", "f32", true) } }
			};
		};
		const auto ok = chain(true);
		EXPECT_TRUE(checkStageChain(ok).empty());

		const auto broken = chain(false);
		const auto diags = checkStageChain(broken);
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(diags[0].message.contains("'level'")) << diags[0].message;
	}
}
