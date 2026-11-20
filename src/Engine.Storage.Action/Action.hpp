#pragma once

#include <Engine.Reflect.Utils/Error/IllegalArgumentError.hpp>
#include <Engine.Reflect.Utils/Error/IllegalTypeError.hpp>

#include "Access/AccessError.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::storage {
	class Action {
	public:
		using this_type = Action;

		using pre_error_type = Variant<IllegalTypeError>;
		using acq_error_type = Variant<AccessError, IllegalArgumentError, IllegalTypeError>;
	};
}
