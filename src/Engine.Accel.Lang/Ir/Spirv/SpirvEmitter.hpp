#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "../Diagnostic.hpp"
#include "../Layout.hpp"
#include "../Module.hpp"
#include "../Tier.hpp"

namespace hg::engine::accel::lang::ir::spirv {
	/* `words` is empty whenever `diagnostics` holds an error; warnings come with the words */
	struct EmitResult {
		Vector<u32> words;
		Diagnostics diagnostics;
	};

	/**
	 * One SPIR-V module for the entry point `main` of a reference-lowered module; only declarations reached from `main` are emitted.
	 * `Tier::eBuild` runs the spirv-opt performance recipe (an optimizer failure ships the unoptimized module with a warning).
	 * From `Tier::eDev` on the module stays unoptimized, user names are kept and the result is validated.
	 */
	[[nodiscard]] EmitResult emit(ref<const Module> lowered_, ref<const Layout> layout_, Tier tier_);
}
