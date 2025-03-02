#pragma once

#include <functional>
#include <Engine.Common/Wrapper.hpp>

namespace hg::editor::ui::rx {
	template <typename Type_>
	struct Subscriber {
		std::function<void(ref<const Type_>)> invoke;
	};

	template <typename Type_>
	Subscriber(std::function<void(ref<const Type_>)>) -> Subscriber<Type_>;
}
