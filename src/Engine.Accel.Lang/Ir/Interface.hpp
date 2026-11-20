#pragma once

#include "Module.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang::ir {
	/**
	 * One binding of a module's interface. `type` indexes the module's type table.
	 */
	struct InterfaceEntry {
		String id;
		TypeRef type;
		BindMode mode = BindMode::eForward;
		BindClass cls = BindClass::eNone;
		bool implicit = false;
		bool input = true;
		/* per-vertex input of geometry/tessellation; `type` is the element type */
		bool arrayed = false;
		/* module-independent type description (type indices differ per module); chain check only */
		String typeName;
		/* per-patch forward output of tess_ctrl / input of tess_eval */
		bool patch = false;
		/* the declaration's span (`NoLocation` when implicit); never compared, hashed or keyed */
		diag::Location at {};

		[[nodiscard]] bool operator==(ref<const InterfaceEntry> other_) const noexcept {
			const auto key = [](ref<const InterfaceEntry> e_) {
				return std::tie(e_.id, e_.type.index, e_.mode, e_.cls, e_.implicit, e_.input, e_.arrayed, e_.typeName, e_.patch);
			};
			return key(*this) == key(other_);
		}
	};

	/**
	 * Resolve order step 4: every binding of a resolved and sliced module (conditions are not evaluated), sorted by
	 * (`input` first, `id`), ties by mode, class, implicit and type index. Implicit outputs (no GLSL variable) are
	 * entries too. Nothing is validated here: a bind entry without class (`eNone`, the parser does not infer one)
	 * is listed as is; `assignLayout` skips it, so the caller's contract check must report it.
	 */
	[[nodiscard]] Vector<InterfaceEntry> computeInterface(ref<const Module> mod_);

	/**
	 * The stage-facing interface of a lowered module: its entries (`computeInterface`), the compute workgroup size
	 * (`{1, 1, 1}` for other stages) and the spec ids of its specialization constants in declaration order. The
	 * numbers (locations, bindings, constant ids) stay in `Layout`.
	 */
	struct InterfaceDesc {
		EntryKind stage = EntryKind::eFragment;
		Vector<InterfaceEntry> entries;
		std::array<u32, 3> workgroupSize { 1uL, 1uL, 1uL };
		Vector<String> specConstantIds;

		[[nodiscard]] bool operator==(const InterfaceDesc&) const noexcept = default;
	};

	[[nodiscard]] InterfaceDesc describeInterface(ref<const Module> mod_);
}
