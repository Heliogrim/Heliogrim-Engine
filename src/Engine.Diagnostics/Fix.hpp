#pragma once

#include <Engine.Common/__macro.hpp>

#include "Location.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::diag {
	enum class Applicability : u8 {
		eMachineApplicable,
		eMaybeIncorrect,
		eHasPlaceholders
	};

	struct TextEdit {
		SourceSpan range;
		String replacement;
	};

	struct Fix {
		String title;
		Vector<TextEdit> edits;
		Applicability applicability;
	};
}
