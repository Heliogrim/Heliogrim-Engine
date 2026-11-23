#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Serialization;
#endif

namespace hg {
	class Actor;
	class LogicComponent;
}

namespace hg::engine::serialization {
	template <CompleteType Type_> requires std::derived_from<Type_, LogicComponent>
	struct ComponentScheme;

	template <CompleteType Type_> requires std::derived_from<Type_, Actor>
	struct ActorScheme;

	template <CompleteType Type_> requires std::derived_from<Type_, LogicComponent>
	struct SelectScheme<Type_> {
		using default_type = ComponentScheme<Type_>;
	};

	template <CompleteType Type_> requires std::derived_from<Type_, Actor>
	struct SelectScheme<Type_> {
		using default_type = ActorScheme<Type_>;
	};
}
