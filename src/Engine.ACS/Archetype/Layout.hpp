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

		static constexpr size_t value = Count_;
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
	template <fixed_string Name_, typename Type_, typename... Options_>
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

		static constexpr auto name = Name_;
		using type = Type_;

		static constexpr size_t count = detail::find_option<detail::is_count, Count<1>, Options_...>::type::value;
		using children = typename detail::find_option<detail::is_children, Children<>, Options_...>::type;
	};

	template <typename>
	struct is_slot : std::false_type {};

	template <fixed_string Name_, typename Type_, typename... Options_>
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

	/**
	 * Extend item: slots added as children of the base slot at Path_, one set per element of an array slot.
	 */
	template <fixed_string Path_, typename... Slots_>
	struct At {
		static_assert(Path_.size() > 0uLL, "At needs a slot path; top-level additions go directly into Extend.");
		static_assert((is_slot<Slots_>::value && ...), "At takes Slot<...> additions.");
	};

	/**
	 * Extend item: replaces the component type of the base slot at Path_.
	 *
	 * @tparam NewType_ Must derive from the replaced type. A different size or alignment that moves any base
	 *	slot needs an `Overridable` base.
	 */
	template <fixed_string Path_, typename NewType_>
	struct Override {
		static constexpr auto path = Path_;
		using type = NewType_;
	};

	namespace detail {
		template <typename>
		struct is_extend_item : std::false_type {};

		template <fixed_string Name_, typename Type_, typename... Options_>
		struct is_extend_item<Slot<Name_, Type_, Options_...>> : std::true_type {};

		template <fixed_string Path_, typename... Slots_>
		struct is_extend_item<At<Path_, Slots_...>> : std::true_type {};

		template <fixed_string Path_, typename NewType_>
		struct is_extend_item<Override<Path_, NewType_>> : std::true_type {};
	}

	/**
	 * Derived layout: base layout plus added slots and overrides.
	 *
	 * Memory is `[added slots][base block]`, so every base slot keeps its distance to the actor.
	 *
	 * @tparam BaseLayout_ `ActorLayout`, `Extend` or `Overridable<...>` of either.
	 * @tparam Items_ `Slot` (child of the root), `At<"path", Slot...>` or `Override<"path", Type>`.
	 */
	template <typename BaseLayout_, typename... Items_>
	struct Extend {
		static_assert(
			(detail::is_extend_item<Items_>::value && ...),
			"Extend takes Slot<...>, At<\"path\", Slot...> or Override<\"path\", Type> items."
		);

		using base = BaseLayout_;
	};
}
