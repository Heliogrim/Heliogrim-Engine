#pragma once

#include <cstddef>
#include <type_traits>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Meta/FixedString.hpp>

namespace hg::engine::acs {
	/**
	 * Slot option: fixed element count of a slot (default 1).
	 */
	template <size_t Count_>
	struct Count {
		static_assert(Count_ > 0uLL, "A slot needs at least one element.");
		static_assert(Count_ <= 0xFFFFuLL, "Slot element count exceeds the 16 bit array index.");

		constexpr static size_t value = Count_;
	};

	/**
	 * Slot option: explicit child marker; the nesting is the component parent hierarchy.
	 */
	template <typename... Slots_>
	struct Children {};

	namespace detail {
		template <typename>
		struct is_count : std::false_type {};

		template <size_t Count_>
		struct is_count<Count<Count_>> : std::true_type {};

		template <typename>
		struct is_children : std::false_type {};

		template <typename... Slots_>
		struct is_children<Children<Slots_...>> : std::true_type {};

		template <template <typename> typename Kind_, typename Default_, typename... Options_>
		struct find_option {
			using type = Default_;
		};

		template <template <typename> typename Kind_, typename Default_, typename First_, typename... Rest_>
		struct find_option<Kind_, Default_, First_, Rest_...> {
			using type = std::conditional_t<
				Kind_<First_>::value,
				First_,
				typename find_option<Kind_, Default_, Rest_...>::type
			>;
		};
	}

	/**
	 * A named component slot of an archetype layout.
	 *
	 * @tparam Name_ Slot name, used for compile-time lookup only.
	 * @tparam Type_ Component type stored in the slot.
	 * @tparam Options_ `Count<N>` and/or `Children<Slot...>`, matched by kind, not position.
	 */
	template <FixedString Name_, typename Type_, typename... Options_>
	struct Slot {
		static_assert(Name_.size() > 0uLL, "Slot names must not be empty.");
		static_assert(Name_.view().find('/') == std::string_view::npos, "Slot names must not contain '/'.");
		static_assert(
			((detail::is_count<Options_>::value || detail::is_children<Options_>::value) && ...),
			"Slot options must be Count<N> or Children<Slot...>."
		);
		static_assert(
			(static_cast<size_t>(detail::is_count<Options_>::value) + ... + 0uLL) <= 1uLL,
			"Slot takes at most one Count option."
		);
		static_assert(
			(static_cast<size_t>(detail::is_children<Options_>::value) + ... + 0uLL) <= 1uLL,
			"Slot takes at most one Children option."
		);

		constexpr static auto name = Name_;
		using type = Type_;

		constexpr static size_t count = detail::find_option<detail::is_count, Count<1>, Options_...>::type::value;
		using children = typename detail::find_option<detail::is_children, Children<>, Options_...>::type;
	};

	template <typename>
	struct is_slot : std::false_type {};

	template <FixedString Name_, typename Type_, typename... Options_>
	struct is_slot<Slot<Name_, Type_, Options_...>> : std::true_type {};

	/**
	 * Archetype layout with exactly one root slot (the root component).
	 */
	template <typename RootSlot_>
	struct ActorLayout {
		static_assert(is_slot<RootSlot_>::value, "ActorLayout takes exactly one root Slot.");
		static_assert(RootSlot_::count == 1uLL, "The root slot cannot be an array.");

		using root = RootSlot_;
	};
}
