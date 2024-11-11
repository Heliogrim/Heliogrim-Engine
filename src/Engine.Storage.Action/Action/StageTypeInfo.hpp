#pragma once

#include <Engine.Reflect/PartialTypeInfo.hpp>

namespace hg::engine::storage {
	struct StageTypeInfo {
		refl::PartialTypeInfo storage {};
		refl::FullTypeInfo resource {};
		/**/
		refl::FullTypeInfo input {};
	};
}
