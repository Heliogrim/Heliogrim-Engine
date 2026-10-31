#include "pch.h"

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
}
