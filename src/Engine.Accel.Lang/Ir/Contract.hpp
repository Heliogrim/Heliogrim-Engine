#pragma once

#include "Diagnostic.hpp"
#include "Interface.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang::ir {
	/**
	 * One binding the stage (pass) side provides or requires. `cls` `eNone` and an empty `typeName` mean "any";
	 * `at` is where the pass declared it.
	 */
	struct ContractEntry {
		String id;
		BindClass cls = BindClass::eNone;
		String typeName;
		diag::Location at;
	};

	/* a contract entry located at the caller's source line */
	[[nodiscard]] ContractEntry declared(
		String id_,
		const BindClass cls_ = BindClass::eNone,
		String typeName_ = {},
		const std::source_location where_ = std::source_location::current()
	);

	/**
	 * What the stage (pass) side of a shader offers and expects, by binding id.
	 */
	struct StageContract {
		Vector<ContractEntry> provides;
		Vector<ContractEntry> requires_;
	};

	/**
	 * Errors, in interface order then `requires_` order, each naming the id:
	 * - a bind input without class (the parser does not infer one);
	 * - a bind input neither in `materialBindings_` nor provided by the stage;
	 * - a bind input whose class differs from the class the stage provides;
	 * - a stage requirement that no shader output (implicit or not) satisfies, or one of a different type.
	 * `missing-binding-class` and `missing-material-binding` point at the shader entry, `contract-class-mismatch` also
	 * labels the provided entry; `missing-stage-output` points at the requirement; `contract-type-mismatch` points at the
	 * shader output (the requirement when the output is implicit) and labels the requirement.
	 */
	[[nodiscard]] Diagnostics checkContract(
		ref<const Vector<InterfaceEntry>> shader_,
		ref<const StageContract> stage_,
		ref<const Vector<String>> materialBindings_
	);

	/**
	 * Stages in pipeline order. Every forward input of stage N+1 must match a non-implicit forward output of stage N by
	 * id and type (`typeName`; an arrayed input of geometry/tess matches a non-arrayed output by element type).
	 * Stage 0 inputs are vertex attributes and unchecked. Unmatched outputs are fine. Errors point at the input and
	 * label the matching output.
	 */
	[[nodiscard]] Diagnostics checkStageChain(std::span<const std::pair<EntryKind, Vector<InterfaceEntry>>> stages_);
}
