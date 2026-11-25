#include "Desc.hpp"

#include "../Atom/Layout/VerticalLayout.hpp"

import Engine.Common;
import Engine.Reflow;
import std;

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

Desc::Desc(mref<std::function<sptr<Widget>()>> make_, mref<Vector<Desc>> children_, StringView name_) :
	make(::hg::move(make_)),
	children(::hg::move(children_)),
	name(name_) {}

Desc::~Desc() = default;

sptr<Widget> hg::engine::reflow::uikit::mount(mref<Desc> desc_) {
	auto widget = desc_.make();
	widget->setName(desc_.name);

	if (desc_.children.empty()) {
		return widget;
	}

	// shortcut: only the column takes children, extend with a setChild branch once a single-slot description exists.
	const auto column = std::dynamic_pointer_cast<VerticalLayout>(widget);
	::hg::assertrt(column != nullptr);
	for (auto& child : desc_.children) {
		column->addChild(mount(::hg::move(child)));
	}
	return widget;
}
