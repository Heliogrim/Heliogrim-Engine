#pragma once

#include <algorithm>
#include <cstddef>
#include <string_view>

namespace hg {
	/**
	 * Structural string literal wrapper, usable as non-type template parameter.
	 *
	 * @tparam N Array extent including the terminating null character.
	 */
	template <size_t N>
	struct fixed_string {
		constexpr fixed_string(const char (&str)[N]) noexcept {
			std::copy_n(str, N, value);
		}

		[[nodiscard]] constexpr size_t size() const noexcept {
			return N - 1uLL;
		}

		[[nodiscard]] constexpr std::string_view view() const noexcept {
			return std::string_view { value, N - 1uLL };
		}

		template <size_t M>
		[[nodiscard]] constexpr bool operator==(const fixed_string<M>& other) const noexcept {
			return view() == other.view();
		}

		// Public to stay structural (NTTP requirement).
		char value[N];
	};

	template <size_t N>
	fixed_string(const char (&)[N]) -> fixed_string<N>;
}
