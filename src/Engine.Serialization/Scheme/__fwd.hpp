#pragma once

#include <concepts>
#include <type_traits>
#include <Engine.Common/Meta/Type.hpp>

namespace hg {
	class Actor;
	class LogicComponent;
}

namespace hg::engine::serialization {
	template <CompleteType Type_>
	struct DefaultScheme;

	template <CompleteType Type_> requires std::derived_from<Type_, LogicComponent>
	struct ComponentScheme;

	template <CompleteType Type_> requires std::derived_from<Type_, Actor>
	struct ActorScheme;
}
