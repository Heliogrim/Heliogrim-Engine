#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow::theming {
	/**
	 * A sparse overlay over one attribute set: one optional value per slot. Applying it writes engaged slots only.
	 */
	template <class Set_>
	class StyleRule {
	public:
		using this_type = StyleRule<Set_>;
		using set_type = Set_;
		using attributes_type = typename Set_::attributes_type;

	private:
		template <typename Tuple_>
		struct SlotTuple;

		template <typename... Named_>
		struct SlotTuple<std::tuple<Named_...>> {
			using type = std::tuple<Opt<typename decltype(Named_::attribute)::store_type>...>;
		};

		constexpr static std::size_t slotCount = std::tuple_size_v<attributes_type>;

	private:
		typename SlotTuple<attributes_type>::type _slots;

	public:
		template <ref<const StringView> Name_>
		ref<this_type> with(ref<const typename Set_::template GetTypeOf<Name_>> value_) {
			constexpr auto index = Set_::template namedIndex<Name_, 0uLL>();
			static_assert(index != (~0uLL), "Failed to find attribute with name.");
			std::get<index>(_slots) = SomeVal<typename Set_::template GetTypeOf<Name_>>(value_);
			return *this;
		}

	public:
		void applyTo(ref<Set_> set_) const {
			applyTo(set_, std::make_index_sequence<slotCount> {});
		}

		void applyTo(ref<this_type> rule_) const {
			applyTo(rule_, std::make_index_sequence<slotCount> {});
		}

	public:
		[[nodiscard]] static this_type full(ref<const Set_> set_) {
			this_type rule {};
			rule.fill(set_, std::make_index_sequence<slotCount> {});
			return rule;
		}

		/**
		 * Copies only the named slots, so a slot added to the set later is never written by this rule.
		 */
		template <ref<const StringView>... Names_>
		[[nodiscard]] static this_type pick(ref<const Set_> set_) {
			this_type rule {};
			(rule.template with<Names_>(set_.template valueOf<Names_>()), ...);
			return rule;
		}

	private:
		template <std::size_t... Index_>
		void applyTo(ref<Set_> set_, std::index_sequence<Index_...>) const {
			(applySlot<Index_>(set_), ...);
		}

		template <std::size_t... Index_>
		void applyTo(ref<this_type> rule_, std::index_sequence<Index_...>) const {
			((std::get<Index_>(_slots).has_value() ? (void)(std::get<Index_>(rule_._slots) = std::get<Index_>(_slots)) : void()), ...);
		}

		template <std::size_t Index_>
		void applySlot(ref<Set_> set_) const {
			if (std::get<Index_>(_slots).has_value()) {
				set_.template update<std::tuple_element_t<Index_, attributes_type>::name>(std::get<Index_>(_slots).value());
			}
		}

		template <std::size_t... Index_>
		void fill(ref<const Set_> set_, std::index_sequence<Index_...>) {
			((std::get<Index_>(_slots) = SomeVal<typename Set_::template GetTypeOf<std::tuple_element_t<Index_, attributes_type>::name>>(
				set_.template valueOf<std::tuple_element_t<Index_, attributes_type>::name>()
			)), ...);
		}
	};
}
