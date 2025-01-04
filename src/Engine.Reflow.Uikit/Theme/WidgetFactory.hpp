#pragma once

#include <concepts>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>

/**/
#include "../Atom/__fwd.hpp"
#include "../Molecule/__fwd.hpp"
#include "../Organism/__fwd.hpp"

/**/
#include "../Atom/Layout/BoxLayout.hpp"
#include "../Atom/Layout/HorizontalLayout.hpp"
#include "../Atom/Layout/UniformGridLayout.hpp"
#include "../Atom/Layout/VerticalLayout.hpp"

namespace hg::engine::reflow::uikit {
	template <class WidgetType_>
	concept HasStyleAttributes = requires {
			typename WidgetType_::Attributes;
		} && requires(ref<WidgetType_> obj_) {
			{ obj_.attr } -> std::same_as<ref<typename WidgetType_::Attributes>>;
		} && requires(ref<const WidgetType_> obj_) {
			{ obj_.attr } -> std::same_as<ref<const typename WidgetType_::Attributes>>;
		};

	template <typename StyleFnType_, typename WidgetType_>
	concept IsStyleFnFor = std::is_invocable_v<StyleFnType_, ref<typename WidgetType_::Attributes>>;
}

namespace hg::engine::reflow::uikit {
	class WidgetFactory {
	public:
		using this_type = WidgetFactory;

	public:
		virtual ~WidgetFactory() noexcept = default;

	public:
		template <std::derived_from<Widget> WidgetType_>
		[[nodiscard]] SharedPtr<WidgetType_> make() const {
			return make_sptr<WidgetType_>();
		}

		template <std::derived_from<Widget> WidgetType_> requires HasStyleAttributes<WidgetType_>
		[[nodiscard]] SharedPtr<WidgetType_> makeWith(typename WidgetType_::Attributes&& attr_) const {
			auto widget = make_sptr<WidgetType_>();
			widget->attr = ::hg::move(attr_);
			return widget;
		}

		template <std::derived_from<Widget> WidgetType_> requires HasStyleAttributes<WidgetType_>
		[[nodiscard]] SharedPtr<WidgetType_> makeWith(auto&& styleFn_) const requires IsStyleFnFor<decltype(styleFn_), WidgetType_> {
			auto widget = make_sptr<WidgetType_>();
			::hg::forward<decltype(styleFn_)>(styleFn_)(widget->attr);
			return widget;
		}

	public:
		[[nodiscard]] virtual SharedPtr<BoxLayout> makeBoxLayout() const = 0;

		[[nodiscard]] virtual SharedPtr<BoxLayout> makeBoxLayout(BoxLayout::Attributes&& attr_) const = 0;

		[[nodiscard]] virtual SharedPtr<HorizontalLayout> makeHorizontalLayout() const = 0;

		[[nodiscard]] virtual SharedPtr<HorizontalLayout> makeHorizontalLayout(HorizontalLayout::Attributes&& attr_) const = 0;

		[[nodiscard]] virtual SharedPtr<UniformGridLayout> makeUniformGridLayout() const = 0;

		[[nodiscard]] virtual SharedPtr<UniformGridLayout> makeUniformGridLayout(UniformGridLayout::Attributes&& attr_) const = 0;

		[[nodiscard]] virtual SharedPtr<VerticalLayout> makeVerticalLayout() const = 0;

		[[nodiscard]] virtual SharedPtr<VerticalLayout> makeVerticalLayout(VerticalLayout::Attributes&& attr_) const = 0;

	public:
		[[nodiscard]] virtual SharedPtr<Image> makeImage() const = 0;

		[[nodiscard]] virtual SharedPtr<Text> makeText() const = 0;
	};

	/**/

	template <>
	inline SharedPtr<BoxLayout> WidgetFactory::make<BoxLayout>() const {
		return makeBoxLayout();
	}

	template <>
	inline SharedPtr<BoxLayout> WidgetFactory::makeWith(BoxLayout::Attributes&& attr_) const {
		return makeBoxLayout(::hg::move(attr_));
	}

	template <>
	inline SharedPtr<HorizontalLayout> WidgetFactory::make<HorizontalLayout>() const {
		return makeHorizontalLayout();
	}

	template <>
	inline SharedPtr<HorizontalLayout> WidgetFactory::makeWith(HorizontalLayout::Attributes&& attr_) const {
		return makeHorizontalLayout(::hg::move(attr_));
	}

	template <>
	inline SharedPtr<UniformGridLayout> WidgetFactory::make<UniformGridLayout>() const {
		return makeUniformGridLayout();
	}

	template <>
	inline SharedPtr<UniformGridLayout> WidgetFactory::makeWith(UniformGridLayout::Attributes&& attr_) const {
		return makeUniformGridLayout(::hg::move(attr_));
	}

	template <>
	inline SharedPtr<VerticalLayout> WidgetFactory::make<VerticalLayout>() const {
		return makeVerticalLayout();
	}

	template <>
	inline SharedPtr<VerticalLayout> WidgetFactory::makeWith(VerticalLayout::Attributes&& attr_) const {
		return makeVerticalLayout(::hg::move(attr_));
	}
}
