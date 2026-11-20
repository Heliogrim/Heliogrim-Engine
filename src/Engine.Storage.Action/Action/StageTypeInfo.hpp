#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflect;
#endif

namespace hg::engine::storage {
	struct StageTypeInfo {
		refl::PartialTypeInfo storage {};
		refl::FullTypeInfo resource {};
		/**/
		refl::FullTypeInfo input {};
	};
}
