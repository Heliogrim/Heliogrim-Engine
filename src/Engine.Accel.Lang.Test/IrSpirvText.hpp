#pragma once

#include <gtest/gtest.h>
#include <spirv-tools/libspirv.hpp>

/**/

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Lang;
import std;
#endif

namespace AccelLang {
	struct SpirvText {
		hg::engine::accel::lang::ir::Diagnostics diagnostics;
		/* disassembly of the validated module; empty when there are diagnostics */
		hg::String text;
	};

	/* lowers references, lays out and emits at the Dev tier (which validates); a parsed module goes in */
	[[nodiscard]] inline SpirvText spirvOf(hg::ref<const hg::engine::accel::lang::ir::Module> parsed_) {
		using namespace hg::engine::accel::lang::ir;
		auto lowered = lowerReferences(parsed_);
		if (not lowered.has_value()) {
			return SpirvText { std::move(lowered).error(), {} };
		}

		auto layout = assignLayout(computeInterface(*lowered), lowered->types, {});
		layout.specConstants = assignSpecConstants(lowered->constants);
		auto res = spirv::emit(*lowered, layout, Tier::eDev);
		if (not res.diagnostics.empty()) {
			return SpirvText { std::move(res.diagnostics), {} };
		}

		spvtools::SpirvTools tools { SPV_ENV_VULKAN_1_3 };
		std::string text {};
		EXPECT_TRUE(tools.Disassemble(res.words, &text, SPV_BINARY_TO_TEXT_OPTION_NO_HEADER | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES));
		return SpirvText { {}, hg::String { text } };
	}

	/* disassembly of a module that must emit cleanly */
	[[nodiscard]] inline hg::String spirvTextOf(hg::ref<const hg::engine::accel::lang::ir::Module> parsed_) {
		auto out = spirvOf(parsed_);
		for (const auto& diagnostic : out.diagnostics) {
			ADD_FAILURE() << diagnostic.message();
		}
		EXPECT_FALSE(out.text.empty());
		return std::move(out.text);
	}
}
