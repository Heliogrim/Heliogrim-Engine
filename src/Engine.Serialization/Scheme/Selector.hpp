#pragma once

#include "__fwd.hpp"

namespace hg::engine::serialization {
	template <CompleteType Type_>
	struct SelectScheme {
		using default_type = DefaultScheme<Type_>;
	};

	template <CompleteType Type_> requires std::derived_from<Type_, LogicComponent>
	struct SelectScheme<Type_> {
		using default_type = ComponentScheme<Type_>;
	};

	template <CompleteType Type_> requires std::derived_from<Type_, Actor>
	struct SelectScheme<Type_> {
		using default_type = ActorScheme<Type_>;
	};
}
