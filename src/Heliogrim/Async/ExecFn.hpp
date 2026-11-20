#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg {
	using execute_function_type = std::function<void()>;
	using execute_function_pointer_type = nular_fnc<void>;
}
