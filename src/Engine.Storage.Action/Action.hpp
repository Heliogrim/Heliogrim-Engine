#pragma once

#include <Engine.Common/Variant.hpp>
#include <Engine.Reflect.Utils/Error/IllegalArgumentError.hpp>
#include <Engine.Reflect.Utils/Error/IllegalTypeError.hpp>

#include "Access/AccessError.hpp"

namespace hg::engine::storage {
	class Action {
	public:
		using this_type = Action;

		using pre_error_type = Variant<IllegalTypeError>;
		using acq_error_type = Variant<AccessError, IllegalArgumentError, IllegalTypeError>;
	};
}
