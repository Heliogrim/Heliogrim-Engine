#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::editor::ui::rx {
	struct Id {
		u16 value;
	};
}

namespace std {
	template <>
	struct less<::hg::editor::ui::rx::Id> {
		[[nodiscard]] constexpr auto operator()(
			::hg::ref<const ::hg::editor::ui::rx::Id> left_,
			::hg::ref<const ::hg::editor::ui::rx::Id> right_
		) const noexcept {
			return left_.value < right_.value;
		}
	};
}
