#pragma once

#include <Engine.Common/Types.hpp>

namespace hg {
	struct LayoutError {
		enum class Kind : u8 {
			// Header, size or fixup table does not fit the target.
			eMismatch,
			// A checked byte holds bits outside its mask, or a type's check hook rejected its payload.
			eInvalidValue
		};

		Kind kind;
		// Byte offset into the layout input.
		u64 offset;
	};
}
