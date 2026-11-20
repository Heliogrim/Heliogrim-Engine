#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::ui::rx {
	template <typename Type_>
	struct Subscriber {
		std::function<void(ref<const Type_>)> invoke;
	};

	template <typename Type_>
	Subscriber(std::function<void(ref<const Type_>)>) -> Subscriber<Type_>;
}
