#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Diagnostic.hpp"
#include "Interface.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * What the stage (pass) side of a shader offers and expects, by binding id.
	 */
	struct StageContract {
		Vector<String> provides;
		Vector<String> requires_;
	};

	/**
	 * Errors, in interface order then `requires_` order, each naming the id:
	 * - a bind input without class (the parser does not infer one);
	 * - a bind input neither in `materialBindings_` nor provided by the stage;
	 * - a stage requirement that no shader output (implicit or not) satisfies.
	 */
	[[nodiscard]] Diagnostics checkContract(
		ref<const Vector<InterfaceEntry>> shader_,
		ref<const StageContract> stage_,
		ref<const Vector<String>> materialBindings_
	);
}
