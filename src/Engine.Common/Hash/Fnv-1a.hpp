#pragma once

#include <span>
#include "../String.hpp"
#include "../Types.hpp"
#include "../Wrapper.hpp"
#include "../__macro.hpp"

namespace hg::hash {
	namespace {
		START_SUPPRESS_WARNINGS
		/**
		 * Fnv-1a 86
		 *
		 * @author Julius
		 * @date 06.09.2020
		 *
		 * @param  str_ The.
		 * @param  count_ Number of.
		 *
		 * @returns A type_id.
		 */
		constexpr u64 fnv1a_86(const char* str_, const std::size_t count_) {
			u64 acc = 2166136261uLL;
			for (size_t idx = 0; idx < count_; ++idx) {
				const auto tmp = acc ^ static_cast<cref<u64>>(str_[idx]);
				acc = tmp * 16777619uLL;
			}
			return acc;
		}

		STOP_SUPPRESS_WARNINGS
	}

	constexpr u64 operator ""_fnv1a(const char* str_, const std::size_t count_) noexcept {
		return fnv1a_86(str_, count_);
	}

	[[nodiscard]] constexpr u64 fnv1a(cref<string_view> src_) noexcept {
		return fnv1a_86(src_.data(), src_.length());
	}

	[[nodiscard]] constexpr u64 fnv1a(cref<string> src_) noexcept {
		return fnv1a_86(src_.data(), src_.length());
	}

	[[nodiscard]] constexpr u64 fnv1a(const std::span<char> src_) noexcept {
		return fnv1a_86(src_.data(), src_.size());
	}
}

namespace hg::hasher {
	template <typename Type_>
	struct fnv1a {
		[[nodiscard]] constexpr size_t operator()(cref<Type_> val_) const noexcept {
			return ::hg::hash::fnv1a(val_);
		}
	};
}
