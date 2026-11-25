#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflow;
#endif

namespace hg::engine::reflow::uikit {
	template <typename DataAttributeType_>
	struct AtomDataAttribute {
		using type = DataAttributeType_;
		DataAttributeType_ dataAttributes;
	};

	template <>
	struct AtomDataAttribute<void> {
		using type = void;
	};

	template <typename LayoutAttributeType_>
	struct AtomLayoutAttribute {
		using type = LayoutAttributeType_;
		LayoutAttributeType_ layoutAttributes;
	};

	template <>
	struct AtomLayoutAttribute<void> {
		using type = void;
	};

	template <typename StyleAttributeType_>
	struct AtomStyleAttribute {
		using type = StyleAttributeType_;
		StyleAttributeType_ styleAttributes;
	};

	template <>
	struct AtomStyleAttribute<void> {
		using type = void;
	};

	template <typename DataAttributeType_, typename LayoutAttributeType_, typename StyleAttributeType_>
	struct Atom :
		AtomDataAttribute<DataAttributeType_>,
		AtomLayoutAttribute<LayoutAttributeType_>,
		AtomStyleAttribute<StyleAttributeType_> {
	public:
		using this_type = Atom<DataAttributeType_, LayoutAttributeType_, StyleAttributeType_>;

		using atom_data_attr = AtomDataAttribute<DataAttributeType_>;
		using atom_layout_attr = AtomLayoutAttribute<LayoutAttributeType_>;
		using atom_style_attr = AtomStyleAttribute<StyleAttributeType_>;

		static_assert(
			uniqueSlotNames<DataAttributeType_, LayoutAttributeType_, StyleAttributeType_>,
			"Slot names must be unique across the data, layout and style sets."
		);

	private:
		template <ref<const StringView> Name_>
		using SlotSet = std::conditional_t<
			hasSlot<DataAttributeType_, Name_>,
			DataAttributeType_,
			std::conditional_t<hasSlot<LayoutAttributeType_, Name_>, LayoutAttributeType_, StyleAttributeType_>
		>;

		template <ref<const StringView> Name_, class Self_>
		[[nodiscard]] static auto& slotSet(Self_& self_) noexcept {
			static_assert(hasSlot<SlotSet<Name_>, Name_>, "Failed to find attribute with name.");
			if constexpr (hasSlot<DataAttributeType_, Name_>) {
				return self_.dataAttributes;

			} else if constexpr (hasSlot<LayoutAttributeType_, Name_>) {
				return self_.layoutAttributes;

			} else {
				return self_.styleAttributes;
			}
		}

		template <ref<const StringView> Name_>
		static bool assign(auto& set_, auto&& next_) noexcept {
			if constexpr (requires { set_.attributeSets; }) {
				return set_.template set<Name_>(::hg::forward<decltype(next_)>(next_));

			} else {
				return set_.template update<Name_>(::hg::forward<decltype(next_)>(next_));
			}
		}

	public:
		template <ref<const StringView> Name_>
		using SlotType = typename SlotSet<Name_>::template GetTypeOf<Name_>;

		template <ref<const StringView> Name_>
		[[nodiscard]] decltype(auto) get() const noexcept {
			const auto& set_ = slotSet<Name_>(*this);
			if constexpr (requires { set_.attributeSets; }) {
				return set_.template get<Name_>();

			} else {
				return set_.template valueOf<Name_>();
			}
		}

		template <ref<const StringView> Name_>
		bool set(ref<const SlotType<Name_>> next_) noexcept {
			return assign<Name_>(slotSet<Name_>(*this), next_);
		}

		template <ref<const StringView> Name_>
		bool set(mref<SlotType<Name_>> next_) noexcept {
			return assign<Name_>(slotSet<Name_>(*this), ::hg::move(next_));
		}

	public:
		void bindAttributeOwner(nmpt<Widget> owner_) noexcept {
			const auto bind = [owner_]<typename Set_>(Set_& set_) {
				if constexpr (requires { set_.attributeSets; }) {
					std::apply([owner_](auto&... sets_) { (sets_.bindOwner(owner_), ...); }, set_.attributeSets);
				} else {
					set_.bindOwner(owner_);
				}
			};
			if constexpr (not std::is_void_v<typename atom_data_attr::type>) {
				bind(atom_data_attr::dataAttributes);
			}
			if constexpr (not std::is_void_v<typename atom_layout_attr::type>) {
				bind(atom_layout_attr::layoutAttributes);
			}
			if constexpr (not std::is_void_v<typename atom_style_attr::type>) {
				bind(atom_style_attr::styleAttributes);
			}
		}

		template <typename ReturnType_ = typename atom_data_attr::type>
			requires (not std::is_void_v<typename atom_data_attr::type>)
		[[nodiscard]] ref<const ReturnType_> getDataAttributes() const noexcept {
			return atom_data_attr::dataAttributes;
		}

		template <typename ReturnType_ = typename atom_data_attr::type>
			requires (not std::is_void_v<typename atom_data_attr::type>)
		[[nodiscard]] ref<ReturnType_> getDataAttributes() noexcept {
			return atom_data_attr::dataAttributes;
		}

		template <typename ReturnType_ = typename atom_layout_attr::type>
			requires (not std::is_void_v<typename atom_layout_attr::type>)
		[[nodiscard]] ref<const ReturnType_> getLayoutAttributes() const noexcept {
			return atom_layout_attr::layoutAttributes;
		}

		template <typename ReturnType_ = typename atom_layout_attr::type>
			requires (not std::is_void_v<typename atom_layout_attr::type>)
		[[nodiscard]] ref<ReturnType_> getLayoutAttributes() noexcept {
			return atom_layout_attr::layoutAttributes;
		}

		template <typename ReturnType_ = typename atom_style_attr::type>
			requires (not std::is_void_v<typename atom_style_attr::type>)
		[[nodiscard]] ref<const ReturnType_> getStyleAttributes() const noexcept {
			return atom_style_attr::styleAttributes;
		}

		template <typename ReturnType_ = typename atom_style_attr::type>
			requires (not std::is_void_v<typename atom_style_attr::type>)
		[[nodiscard]] ref<ReturnType_> getStyleAttributes() noexcept {
			return atom_style_attr::styleAttributes;
		}
	};
}
