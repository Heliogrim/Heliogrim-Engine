#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>

#include "Location.hpp"

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
