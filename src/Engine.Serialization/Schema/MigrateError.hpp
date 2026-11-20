#pragma once

#include <Engine.Common/String.hpp>

namespace hg::engine::serialization {
	struct MigrateError {
		String message;
	};
}
