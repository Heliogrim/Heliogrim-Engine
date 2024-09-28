#pragma once

#include <cmath>

#include "../Forward.hpp"
#include "../__macro.hpp"

#if defined(_MSC_VER)

namespace hg::math {
	inline auto floorf(auto&& val_) noexcept {
		return ::std::floorf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto ceilf(auto&& val_) noexcept {
		return ::std::ceilf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto sqrtf(auto&& val_) noexcept {
		return ::std::sqrtf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto logf(auto&& val_) noexcept {
		return ::std::logf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto log2f(auto&& val_) noexcept {
		return ::std::log2f(::hg::forward<decltype(val_)>(val_));
	}

	inline auto powf(auto&& base_, auto&& exp_) noexcept {
		return ::std::powf(::hg::forward<decltype(base_)>(base_), ::hg::forward<decltype(exp_)>(exp_));
	}

	inline auto sinf(auto&& val_) noexcept {
		return ::std::sinf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto asinf(auto&& val_) noexcept {
		return ::std::asinf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto cosf(auto&& val_) noexcept {
		return ::std::cosf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto acosf(auto&& val_) noexcept {
		return ::std::acosf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto tanf(auto&& val_) noexcept {
		return ::std::tanf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto atanf(auto&& val_) noexcept {
		return ::std::atanf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto atan2f(auto&& y_, auto&& x_) noexcept {
		return ::std::atan2f(::hg::forward<decltype(y_)>(y_), ::hg::forward<decltype(x_)>(x_));
	}
}

#else

namespace hg::math {
	inline auto floorf(auto&& val_) noexcept {
		return ::floorf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto ceilf(auto&& val_) noexcept {
		return ::ceilf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto sqrtf(auto&& val_) noexcept {
		return ::sqrtf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto logf(auto&& val_) noexcept {
		return ::logf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto log2f(auto&& val_) noexcept {
		return ::log2f(::hg::forward<decltype(val_)>(val_));
	}

	inline auto powf(auto&& base_, auto&& exp_) noexcept {
		return ::powf(::hg::forward<decltype(base_)>(base_), ::hg::forward<decltype(exp_)>(exp_));
	}

	inline auto sinf(auto&& val_) noexcept {
		return ::sinf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto asinf(auto&& val_) noexcept {
		return ::asinf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto cosf(auto&& val_) noexcept {
		return ::cosf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto acosf(auto&& val_) noexcept {
		return ::acosf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto tanf(auto&& val_) noexcept {
		return ::tanf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto atanf(auto&& val_) noexcept {
		return ::atanf(::hg::forward<decltype(val_)>(val_));
	}

	inline auto atan2f(auto&& y_, auto&& x_) noexcept {
		return ::atan2f(::hg::forward<decltype(y_)>(y_), ::hg::forward<decltype(x_)>(x_));
	}
}

#endif
