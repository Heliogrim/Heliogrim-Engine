#include "Components.hpp"

import Engine.Common;
import Engine.Reflow;
import std;

using namespace hg::engine::reflow::uikit::desc;
using namespace hg::engine::reflow;
using namespace hg;

using hg::engine::reflow::uikit::Desc;

namespace {
	template <ref<const StringView> Name_, class Widget_, class Value_>
	void setIf(ref<Widget_> widget_, ref<const Opt<Value_>> value_);
}

Desc Column::build(mref<Vector<Desc>> children_) const {
	return Desc {
		[self = *this]() -> sptr<Widget> {
			auto widget = make_sptr<uikit::VerticalLayout>();
			setIf<attr::BoxLayout::minWidth>(*widget, self.minWidth);
			setIf<attr::BoxLayout::minHeight>(*widget, self.minHeight);
			setIf<attr::BoxLayout::padding>(*widget, self.padding);
			setIf<attr::FlexLayout::colGap>(*widget, self.colGap);
			setIf<attr::FlexLayout::rowGap>(*widget, self.rowGap);
			if (self.ref != nullptr) {
				*self.ref = widget;
			}
			return widget;
		},
		::hg::move(children_),
		name
	};
}

Desc Label::build(mref<Vector<Desc>> children_) const {
	return Desc {
		[self = *this]() -> sptr<Widget> {
			auto widget = make_sptr<uikit::Text>();
			setIf<attr::BoxLayout::minWidth>(*widget, self.minWidth);
			setIf<attr::BoxLayout::minHeight>(*widget, self.minHeight);
			setIf<attr::BoxLayout::padding>(*widget, self.padding);
			setIf<attr::TextData::text>(*widget, self.text);
			if (self.ref != nullptr) {
				*self.ref = widget;
			}
			return widget;
		},
		::hg::move(children_),
		name
	};
}

Desc hg::engine::reflow::uikit::desc::Surface::build(mref<Vector<Desc>> children_) const {
	return Desc {
		[self = *this]() -> sptr<Widget> {
			auto widget = make_sptr<uikit::Surface>();
			setIf<attr::BoxLayout::minWidth>(*widget, self.minWidth);
			setIf<attr::BoxLayout::minHeight>(*widget, self.minHeight);
			setIf<attr::BoxLayout::padding>(*widget, self.padding);
			setIf<attr::PaintStyle::tint>(*widget, self.tint);
			setIf<attr::PaintStyle::borderRadius>(*widget, self.borderRadius);
			// The engaged style members also form the instance rule, so a restyle keeps them over the component style.
			if (self.tint.has_value()) {
				widget->instance.with<attr::PaintStyle::tint>(*self.tint);
			}
			if (self.borderRadius.has_value()) {
				widget->instance.with<attr::PaintStyle::borderRadius>(*self.borderRadius);
			}
			widget->component = self.component;
			if (self.ref != nullptr) {
				*self.ref = widget;
			}
			return widget;
		},
		::hg::move(children_),
		name
	};
}

/**/

namespace {
	template <ref<const StringView> Name_, class Widget_, class Value_>
	void setIf(ref<Widget_> widget_, ref<const Opt<Value_>> value_) {
		if (value_.has_value()) {
			widget_.template set<Name_>(*value_);
		}
	}
}
