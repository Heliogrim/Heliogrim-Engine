#pragma once

#include <type_traits>

#include "Sal.hpp"
#include "Types.hpp"
#include "Wrapper.hpp"

namespace hg {
	template <
		typename Ty,
		class UTy = std::underlying_type_t<Ty>>
	struct Flag {
		static_assert(std::is_enum_v<Ty>, "Enumeration required.");

		/** Type of the value */
		using value_type = UTy;

		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 20.12.2020
		 */
		constexpr Flag() noexcept = default;

		/**
		 * Copy constructor
		 *
		 * @author Julius
		 * @date 20.12.2020
		 *
		 * @param  other_ The other.
		 */
		constexpr Flag(const Flag& other_) noexcept = default;

		/**
		 * Move constructor
		 *
		 * @author Julius
		 * @date 20.12.2020
		 *
		 * @param [in,out] other_ The other.
		 */
		Flag(Flag&& other_) noexcept = default;

		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 20.12.2020
		 *
		 * @param  initial_ The initial.
		 */
		constexpr Flag(const Ty& initial_) noexcept :
			unwrap(static_cast<value_type>(initial_)) {}

		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 14.11.2022
		 *
		 * @params initial_ The initial underlying value
		 */
		constexpr Flag(const value_type& initial_) noexcept :
			unwrap(initial_) {}

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 20.12.2020
		 */
		~Flag() noexcept = default;

		/**
		 * Bitwise 'or' assignment operator
		 *
		 * @author Julius
		 * @date 20.12.2020
		 *
		 * @param  other_ The other.
		 *
		 * @returns The result of the operation.
		 */
		Flag& operator |=(const Flag& other_) {
			unwrap |= other_.unwrap;
			return *this;
		}

		/**
		 * Bitwise 'or' assignment operator
		 *
		 * @author Julius
		 * @date 20.12.2020
		 *
		 * @param  other_ The other.
		 *
		 * @returns The result of the operation.
		 */
		constexpr Flag& operator |=(const Ty& other_) {
			unwrap |= static_cast<value_type>(other_);
			return *this;
		}

		/**
		 * Bitwise 'or' operator
		 *
		 * @author Julius
		 * @date 20.12.2020
		 *
		 * @param  other_ The other.
		 *
		 * @returns The result of the operation.
		 */
		Flag operator|(const Flag& other_) const noexcept {
			Flag res { *this };
			return res |= other_;
		}

		/**
		 * Bitwise 'or' operator
		 *
		 * @author Julius
		 * @date 20.12.2020
		 *
		 * @param  other_ The other.
		 *
		 * @returns The result of the operation.
		 */
		Flag operator|(const Ty& other_) const noexcept {
			Flag res { *this };
			return res |= other_;
		}

		/**
		 * Bitwise 'and' operator
		 *
		 * @author Julius
		 * @date 20.12.2020
		 *
		 * @param  other_ The other.
		 *
		 * @returns The result of the operation.
		 */
		bool operator&(const Flag& other_) const noexcept {
			return unwrap & other_.unwrap;
		}

		/**
		 * Bitwise 'and' operator
		 *
		 * @author Julius
		 * @date 20.12.2020
		 *
		 * @param  other_ The other.
		 *
		 * @returns The result of the operation.
		 */
		bool operator&(const Ty& other_) const noexcept {
			return unwrap & static_cast<value_type>(other_);
		}

		/**
		 * Assignment operator
		 *
		 * @author Julius
		 * @date 20.12.2020
		 *
		 * @param  other_ The other.
		 *
		 * @returns A shallow copy of this.
		 */
		Flag operator=(const Ty& other_) const noexcept {
			return Flag { other_ };
		}

		/**
		 * Assignment operator
		 *
		 * @author Julius
		 * @date 20.12.2020
		 *
		 * @param  other_ The other.
		 *
		 * @returns A shallow copy of this.
		 */
		Flag& operator=(const Ty& other_) noexcept {
			unwrap = static_cast<value_type>(other_);
			return *this;
		}

		/**
		 * Copy Assignment operator
		 *
		 * @author Julius
		 * @date 07.11.2021
		 *
		 * @param other_ The other flag.
		 *
		 * @returns A shallow copy of this.
		 */
		ref<Flag> operator=(_In_ cref<Flag> other_) noexcept {

			if (std::addressof(other_) != this) {
				unwrap = other_.unwrap;
			}

			return *this;
		}

		/**
		 * Move Assignment operator
		 *
		 * @author Julius
		 * @date 07.11.2021
		 *
		 * @param other_ The other flag.
		 *
		 * @returns A shallow copy of this.
		 */
		ref<Flag> operator=(_In_ mref<Flag> other_) noexcept {

			if (std::addressof(other_) != this) {
				unwrap = other_.unwrap;
			}

			return *this;
		}

		/**
		 * Equality operator
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  value_ The value.
		 *
		 * @returns True if the parameters are considered equivalent.
		 */
		[[nodiscard]] bool operator==(const Ty& value_) const noexcept {
			return value_ == static_cast<Ty>(unwrap);
		}

		/**
		 * Inequality operator
		 *
		 * @author Julius
		 * @date 18.06.2022
		 *
		 * @param   value_ The value.
		 *
		 * @returns True if the parameters are considered not equivalent.
		 */
		[[nodiscard]] bool operator!=(const Ty& value_) const noexcept {
			return value_ != static_cast<Ty>(unwrap);
		}

		/** The unwrap */
		value_type unwrap;
	};
}

template <typename Ty, class UTy>
hg::Flag<Ty, UTy> operator|(const Ty& left_, const Ty& right_) {
	return hg::Flag<Ty, UTy> { left_ } |= right_;
}
