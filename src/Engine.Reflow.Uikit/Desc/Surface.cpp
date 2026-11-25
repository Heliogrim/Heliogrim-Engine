#include "Surface.hpp"

import Engine.Common;
import Engine.Reflow;
import std;

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

Surface::Surface() :
	Paint(ReflowClassList { "[Surface]"sv }, nullptr),
	component(),
	instance() {}

Surface::~Surface() = default;

string Surface::getTag() const noexcept {
	return std::format(R"(Surface <{:#x}>)", reinterpret_cast<u64>(this));
}

void Surface::onStyleStatesChanged() {
	Paint::onStyleStatesChanged();
	const AttributeBatch batch { *this };
	restyle();
}

void Surface::cascadeContextChange(bool invalidate_) {
	{
		const AttributeBatch batch { *this };
		restyle();
	}
	// Skips Paint's override, which would resolve the legacy class themes on top.
	Widget::cascadeContextChange(invalidate_);
}

void Surface::restyle() {
	// Resolve onto fresh defaults, so a state removed since the last resolve gives its slots back to the base.
	PaintStyleAttributes next {};

	if (getEffectiveProvisioner() != None && not component.empty()) {
		ReflowClassList classes {};
		classes.add(component);
		// No legacy slots: legacy class themes do not apply to a surface, only its component style and instance rule.
		theming::resolveStyle<PaintStyleAttributes>(
			getEffectiveProvisioner()->getContext(),
			classes,
			styleStates(),
			instance,
			next
		);
	} else {
		instance.applyTo(next);
	}

	// Per-slot update marks only the slots whose value changed.
	theming::StyleRule<PaintStyleAttributes>::full(next).applyTo(getStyleAttributes());
}
