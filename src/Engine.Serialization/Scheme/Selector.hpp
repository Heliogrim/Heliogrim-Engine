#pragma once

#include "__fwd.hpp"

namespace hg::engine::serialization {
	template <CompleteType Type_>
	struct SelectScheme {
		using default_type = DefaultScheme<Type_>;
	};
}
