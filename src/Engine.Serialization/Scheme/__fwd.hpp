#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
