#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Module.hpp"

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

		[[nodiscard]] bool operator==(const InterfaceEntry&) const noexcept = default;
	};

	/**
	 * Resolve order step 4: every binding of a resolved and sliced module (conditions are not evaluated), sorted by
	 * (`input` first, `id`), ties by mode, class, implicit and type index. Implicit outputs (no GLSL variable) are
	 * entries too. Nothing is validated here: a bind entry without class (`eNone`, the parser does not infer one)
	 * is listed as is; `assignLayout` skips it, so the caller's contract check must report it.
	 */
	[[nodiscard]] Vector<InterfaceEntry> computeInterface(ref<const Module> mod_);
}
