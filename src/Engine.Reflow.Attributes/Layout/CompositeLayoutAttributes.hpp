#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
import Engine.Common;
#endif

namespace hg::engine::reflow {
	template <class Set_>
	[[nodiscard]] constexpr auto slotNamesOf() noexcept {
		if constexpr (std::is_void_v<Set_>) {
			return std::array<StringView, 0uLL> {};
		} else {
			return Set_::slotNames;
		}
	}

	template <class... Sets_>
	[[nodiscard]] constexpr auto concatSlotNames() noexcept {
		std::array<StringView, (slotNamesOf<Sets_>().size() + ... + 0uLL)> names {};
		auto at = names.begin();
		((at = std::ranges::copy(slotNamesOf<Sets_>(), at).out), ...);
		return names;
	}

	template <class... Sets_>
	constexpr bool uniqueSlotNames = [] {
		constexpr auto names = concatSlotNames<Sets_...>();
		for (std::size_t i = 0uLL; i < names.size(); ++i) {
			for (std::size_t j = i + 1uLL; j < names.size(); ++j) {
				if (names[i] == names[j]) {
					return false;
				}
			}
		}
		return true;
	}();

	/**
	 * True if `Set_` (a plain attribute set or a composite of sets) holds a slot named `Name_`.
	 */
	template <class Set_, ref<const StringView> Name_>
	constexpr bool hasSlot = [] {
		if constexpr (std::is_void_v<Set_>) {
			return false;
		} else if constexpr (requires { typename Set_::attribute_set_tuple; }) {
			return Set_::template setIndexOf<Name_>() != (~0uLL);
		} else {
			return Set_::template namedIndex<Name_, 0uLL>() != (~0uLL);
		}
	}();

	template <class... AttributeSets_>
	struct CompositeLayoutAttributes {
	public:
		using this_type = CompositeLayoutAttributes<AttributeSets_...>;
		using attribute_set_tuple = std::tuple<AttributeSets_...>;

		constexpr static auto slotNames = concatSlotNames<AttributeSets_...>();

		constexpr static bool isCompositeAttributes = true;

	private:
		attribute_set_tuple attributeSets;

	public:
		constexpr CompositeLayoutAttributes() = default;

		constexpr CompositeLayoutAttributes(AttributeSets_... sets_) :
			attributeSets(::hg::move(sets_)...) {}

		constexpr CompositeLayoutAttributes(attribute_set_tuple sets_) :
			attributeSets(::hg::move(sets_)) {}

	public:
		template <class Fn_>
		constexpr void forEachSet(Fn_&& fn_) {
			std::apply([&fn_](auto&... sets_) { (fn_(sets_), ...); }, attributeSets);
		}

		template <class Fn_>
		constexpr void forEachSet(Fn_&& fn_) const {
			std::apply([&fn_](const auto&... sets_) { (fn_(sets_), ...); }, attributeSets);
		}

	public:
		template <ref<const StringView> Name_>
		[[nodiscard]] constexpr static std::size_t setIndexOf() noexcept {
			constexpr bool holds[] { hasSlot<AttributeSets_, Name_>... };
			for (std::size_t i = 0uLL; i < sizeof...(AttributeSets_); ++i) {
				if (holds[i]) {
					return i;
				}
			}
			return (~0uLL);
		}

		template <ref<const StringView> Name_>
		using GetTypeOf = typename std::tuple_element_t<setIndexOf<Name_>(), attribute_set_tuple>::template GetTypeOf<Name_>;

	public:
		template <ref<const StringView> Name_>
		[[nodiscard]] decltype(auto) get() const noexcept {
			return std::get<setIndexOf<Name_>()>(attributeSets).template valueOf<Name_>();
		}

		template <ref<const StringView> Name_>
		bool set(ref<const GetTypeOf<Name_>> next_) noexcept {
			return std::get<setIndexOf<Name_>()>(attributeSets).template update<Name_>(next_);
		}

		template <ref<const StringView> Name_>
		bool set(mref<GetTypeOf<Name_>> next_) noexcept {
			return std::get<setIndexOf<Name_>()>(attributeSets).template update<Name_>(::hg::move(next_));
		}
	};
}
