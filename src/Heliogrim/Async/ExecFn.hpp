#pragma once

#include <Engine.Common/Functional/Function.hpp>

namespace hg {
	using execute_function_type = std::function<void()>;
	using execute_function_pointer_type = nular_fnc<void>;
}
