#pragma once

#include <type_traits>
#include <Engine.Common/Wrapper.hpp>

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

	public:
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
