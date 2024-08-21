#pragma once

#include <cmath>
#include <utility>

#include "../Types.hpp"
#include "../__macro.hpp"

#if defined(_MSC_VER)

namespace hg::math {
	inline auto floorf(auto&& val_) noexcept {
		return ::std::floorf(std::forward<decltype(val_)>(val_));
	}

	inline auto ceilf(auto&& val_) noexcept {
		return ::std::ceilf(std::forward<decltype(val_)>(val_));
	}

	inline auto sqrtf(auto&& val_) noexcept {
		return ::std::sqrtf(std::forward<decltype(val_)>(val_));
	}

	inline auto logf(auto&& val_) noexcept {
		return ::std::logf(std::forward<decltype(val_)>(val_));
	}

	inline auto log2f(auto&& val_) noexcept {
		return ::std::log2f(std::forward<decltype(val_)>(val_));
	}

	inline auto powf(auto&& base_, auto&& exp_) noexcept {
		return ::std::powf(std::forward<decltype(base_)>(base_), std::forward<decltype(exp_)>(exp_));
	}

	inline auto sinf(auto&& val_) noexcept {
		return ::std::sinf(std::forward<decltype(val_)>(val_));
	}

	inline auto asinf(auto&& val_) noexcept {
		return ::std::asinf(std::forward<decltype(val_)>(val_));
	}

	inline auto cosf(auto&& val_) noexcept {
		return ::std::cosf(std::forward<decltype(val_)>(val_));
	}

	inline auto acosf(auto&& val_) noexcept {
		return ::std::acosf(std::forward<decltype(val_)>(val_));
	}

	inline auto tanf(auto&& val_) noexcept {
		return ::std::tanf(std::forward<decltype(val_)>(val_));
	}

	inline auto atanf(auto&& val_) noexcept {
		return ::std::atanf(std::forward<decltype(val_)>(val_));
	}

	inline auto atan2f(auto&& y_, auto&& x_) noexcept {
		return ::std::atan2f(std::forward<decltype(y_)>(y_), std::forward<decltype(x_)>(x_));
	}
}

#else

namespace hg::math {
	inline auto floorf(auto&& val_) noexcept {
		return ::floorf(std::forward<decltype(val_)>(val_));
	}

	inline auto ceilf(auto&& val_) noexcept {
		return ::ceilf(std::forward<decltype(val_)>(val_));
	}

	inline auto sqrtf(auto&& val_) noexcept {
		return ::sqrtf(std::forward<decltype(val_)>(val_));
	}

	inline auto logf(auto&& val_) noexcept {
		return ::logf(std::forward<decltype(val_)>(val_));
	}

	inline auto log2f(auto&& val_) noexcept {
		return ::log2f(std::forward<decltype(val_)>(val_));
	}

	inline auto powf(auto&& base_, auto&& exp_) noexcept {
		return ::powf(std::forward<decltype(base_)>(base_), std::forward<decltype(exp_)>(exp_));
	}

	inline auto sinf(auto&& val_) noexcept {
		return ::sinf(std::forward<decltype(val_)>(val_));
	}

	inline auto asinf(auto&& val_) noexcept {
		return ::asinf(std::forward<decltype(val_)>(val_));
	}

	inline auto cosf(auto&& val_) noexcept {
		return ::cosf(std::forward<decltype(val_)>(val_));
	}

	inline auto acosf(auto&& val_) noexcept {
		return ::acosf(std::forward<decltype(val_)>(val_));
	}

	inline auto tanf(auto&& val_) noexcept {
		return ::tanf(std::forward<decltype(val_)>(val_));
	}

	inline auto atanf(auto&& val_) noexcept {
		return ::atanf(std::forward<decltype(val_)>(val_));
	}

	inline auto atan2f(auto&& y_, auto&& x_) noexcept {
		return ::atan2f(std::forward<decltype(y_)>(y_), std::forward<decltype(x_)>(x_));
	}
}

#endif
