#include "pch.h"

#include <array>
#include <Engine.Accel.Lang/Ir/Contract.hpp>
#include "IrWording.hpp"

using namespace hg::engine::accel::lang::ir;
using namespace hg;
namespace diag = hg::engine::diag;

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
		const StageContract stage { .provides = { declared("view"), declared("block") }, .requires_ = {} };

		const auto diags = checkContract(shader(), stage, {});

		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(diags[0].severity(), Severity::eError);
		expectWording(diags[0]);
		EXPECT_TRUE(diags[0].message().contains("'model'")) << diags[0].message();
	}

	TEST(ContractTest, StageRequirementNotProvidedNamesId) {
		const StageContract stage { .provides = { declared("view"), declared("block") }, .requires_ = { declared("depth"), declared("normal") } };

		const auto diags = checkContract(shader(), stage, { "model" });

		ASSERT_EQ(diags.size(), 1uz);
		expectWording(diags[0]);
		EXPECT_TRUE(diags[0].message().contains("'normal'")) << diags[0].message();
	}

	TEST(ContractTest, SatisfiedContractIsEmpty) {
		// implicit (depth) and non-implicit (color) outputs both satisfy requirements; forward inputs need no binding
		const StageContract stage { .provides = { declared("view") }, .requires_ = { declared("depth"), declared("color") } };

		EXPECT_TRUE(checkContract(shader(), stage, { "model", "block" }).empty());
	}

	TEST(ContractTest, BindWithoutClassNamesId) {
		const auto diags = checkContract({ bind("albedo", BindClass::eNone) }, {}, { "albedo" });

		ASSERT_EQ(diags.size(), 1uz);
		expectWording(diags[0]);
		EXPECT_TRUE(diags[0].message().contains("'albedo'")) << diags[0].message();
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
		expectWording(diags[0]);
		EXPECT_EQ(diags[0].message(), "fragment input 'uv' is 'f32vec2', but the vertex stage outputs 'f32vec3'");
	}

	TEST(ContractTest, ChainMissingInputNamesStages) {
		const std::array<Stage, 2> stages {
			Stage { EntryKind::eGeometry, { fwd("a", "f32", false) } },
			Stage { EntryKind::eFragment, { fwd("world-pos", "f32vec4", true) } }
		};
		const auto diags = checkStageChain(stages);
		ASSERT_EQ(diags.size(), 1uz);
		expectWording(diags[0]);
		EXPECT_EQ(diags[0].message(), "fragment input 'world-pos' has no matching output in the geometry stage");
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
		expectWording(diags[0]);
		EXPECT_TRUE(diags[0].message().contains("'level'")) << diags[0].message();
	}

	namespace {
		[[nodiscard]] diag::Location spanAt(StringView file_, u32 line_) {
			return diag::SourceSpan { .file = String(file_), .begin = 0uL, .end = 0uL, .line = line_, .column = 1uL };
		}

		[[nodiscard]] InterfaceEntry located(InterfaceEntry entry_, u32 line_) {
			entry_.at = spanAt("S", line_);
			return entry_;
		}

		void expectAt(ref<const diag::Location> at_, StringView file_, u32 line_) {
			ASSERT_TRUE(at_.is<diag::SourceSpan>());
			EXPECT_EQ(at_.as<diag::SourceSpan>().file, file_);
			EXPECT_EQ(at_.as<diag::SourceSpan>().line, line_);
		}

		void expectAt(ref<const diag::Location> at_, ref<const diag::Location> expected_) {
			ASSERT_TRUE(expected_.is<diag::SourceSpan>());
			expectAt(at_, expected_.as<diag::SourceSpan>().file, expected_.as<diag::SourceSpan>().line);
		}
	}

	TEST(ContractTest, ClassMismatchPointsAtShaderAndPass) {
		const StageContract stage { .provides = { declared("lights", BindClass::eUniformBuffer) }, .requires_ = {} };

		const auto diags = checkContract({ located(bind("lights", BindClass::eStorageBuffer), 3u) }, stage, {});

		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(StringView { diags[0].code()->key }, "contract-class-mismatch");
		expectWording(diags[0]);
		expectAt(diags[0].primary(), "S", 3u);
		ASSERT_EQ(diags[0].labels().size(), 1uz);
		expectAt(diags[0].labels()[0].at, stage.provides[0].at);
		EXPECT_EQ(diags[0].labels()[0].message, "provided here");
	}

	TEST(ContractTest, TypeMismatchPointsAtShaderAndPass) {
		auto color = located(output("color", false), 5u);
		color.typeName = "f32vec4";
		const StageContract stage { .provides = {}, .requires_ = { declared("color", BindClass::eNone, "f32vec3") } };

		const auto diags = checkContract({ color }, stage, {});

		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(StringView { diags[0].code()->key }, "contract-type-mismatch");
		expectWording(diags[0]);
		expectAt(diags[0].primary(), "S", 5u);
		ASSERT_EQ(diags[0].labels().size(), 1uz);
		expectAt(diags[0].labels()[0].at, stage.requires_[0].at);
		EXPECT_EQ(diags[0].labels()[0].message, "required here");
	}

	TEST(ContractTest, TypedRequirementMatches) {
		auto color = located(output("color", false), 5u);
		color.typeName = "f32vec4";
		const StageContract stage { .provides = {}, .requires_ = { declared("color", BindClass::eNone, "f32vec4") } };

		EXPECT_TRUE(checkContract({ color }, stage, {}).empty());
	}

	TEST(ContractTest, MissingStageOutputPointsAtPass) {
		const StageContract stage { .provides = {}, .requires_ = { declared("depth") } };

		const auto diags = checkContract({ located(output("color", false), 2u) }, stage, {});

		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(StringView { diags[0].code()->key }, "missing-stage-output");
		expectAt(diags[0].primary(), stage.requires_[0].at);
	}

	TEST(ContractTest, MissingMaterialBindingPointsAtShader) {
		const auto diags = checkContract({ located(bind("model", BindClass::eStorageBuffer), 7u) }, {}, {});

		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(StringView { diags[0].code()->key }, "missing-material-binding");
		expectAt(diags[0].primary(), "S", 7u);
	}

	TEST(ContractTest, ChainMismatchPointsAtBothStages) {
		const std::array<Stage, 2> stages {
			Stage { EntryKind::eVertex, { located(fwd("n", "f32vec4", false), 4u) } },
			Stage { EntryKind::eFragment, { located(fwd("n", "f32vec3", true), 9u) } }
		};

		const auto diags = checkStageChain(stages);

		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(StringView { diags[0].code()->key }, "stage-interface-mismatch");
		expectWording(diags[0]);
		expectAt(diags[0].primary(), "S", 9u);
		ASSERT_EQ(diags[0].labels().size(), 1uz);
		expectAt(diags[0].labels()[0].at, "S", 4u);
		EXPECT_EQ(diags[0].labels()[0].message, "output declared here");
	}

	TEST(ContractTest, DeclaredCapturesCallSite) {
		const u32 line = __LINE__ + 1u;
		const auto entry = declared("x");

		ASSERT_TRUE(entry.at.is<diag::SourceSpan>());
		EXPECT_TRUE(StringView { entry.at.as<diag::SourceSpan>().file }.ends_with("IrContractTest.cpp"))
			<< entry.at.as<diag::SourceSpan>().file;
		EXPECT_EQ(entry.at.as<diag::SourceSpan>().line, line);
	}

	TEST(ContractTest, InterfaceEqualityIgnoresLocation) {
		EXPECT_EQ(located(bind("a", BindClass::eTexture), 1u), located(bind("a", BindClass::eTexture), 2u));
		EXPECT_NE(bind("a", BindClass::eTexture), bind("b", BindClass::eTexture));
	}

	TEST(ContractTest, InterfaceLocatesDeclarations) {
		Module mod {};
		mod.name = "M";
		mod.types.emplace_back(TypeDesc {});
		mod.bindings.emplace_back(BindingDecl { .id = "a", .type = TypeRef { 0u }, .loc = SourceLoc { .line = 4uL, .column = 2uL } });

		const auto entries = computeInterface(mod);

		ASSERT_EQ(entries.size(), 1uz);
		expectAt(entries[0].at, "M", 4u);
	}

	TEST(ContractTest, InterfaceImplicitEntryHasNoLocation) {
		Module mod {};
		mod.name = "M";
		mod.types.emplace_back(TypeDesc {});
		mod.bindings.emplace_back(BindingDecl { .id = "a", .type = TypeRef { 0u }, .implicit = true, .loc = SourceLoc { .line = 4uL, .column = 2uL } });

		const auto entries = computeInterface(mod);

		ASSERT_EQ(entries.size(), 1uz);
		EXPECT_TRUE(entries[0].at.is<diag::NoLocation>());
	}

	TEST(ContractTest, InterfaceOutOfRangeSourceHasNoLocation) {
		Module mod {};
		mod.name = "M";
		mod.sources = { "a.asl" };
		mod.types.emplace_back(TypeDesc {});
		mod.bindings.emplace_back(BindingDecl { .id = "a", .type = TypeRef { 0u }, .loc = SourceLoc { .source = 3u, .line = 4uL, .column = 2uL } });

		const auto entries = computeInterface(mod);

		ASSERT_EQ(entries.size(), 1uz);
		EXPECT_TRUE(entries[0].at.is<diag::NoLocation>());
	}
}
