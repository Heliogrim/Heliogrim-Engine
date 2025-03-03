#include "Stack.hpp"

#include <format>

#include <Engine.Reflow/Layout/Style.hpp>

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

uikit::Stack::Stack() :
	Widget(),
	Atom(
		{},
		{
			BoxLayoutAttributes {
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				Padding { 0.F },
				ReflowPlacement::eMiddleCenter
			}
		},
		{}
	) {}

uikit::Stack::~Stack() = default;

string uikit::Stack::getTag() const noexcept {
	return std::format(R"(Stack <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const Children> uikit::Stack::children() const {
	return &_children;
}

void uikit::Stack::addChild(cref<sptr<Widget>> child_) {

	child_->setParent(shared_from_this());
	_children.emplace_back(child_);

	markAsPending();
}

void uikit::Stack::setChild(const u32 idx_, cref<sptr<Widget>> child_) {

	if (idx_ == _children.size()) {
		addChild(child_);
		return;
	}

	if (idx_ > _children.size()) {
		return;
	}

	child_->setParent(shared_from_this());
	_children[idx_]->setParent(nullptr);
	_children[idx_] = child_;

	markAsPending();
}

void uikit::Stack::removeChild(cref<sptr<Widget>> child_) {

	s32 found = -1L;
	for (u32 idx = 0uL; idx < _children.size(); ++idx) {
		if (_children[idx] == child_) {
			found = idx;
			break;
		}
	}

	/**/

	if (found < 0) {
		return;
	}

	const auto iter = _children.begin() + found;
	(*iter)->setParent(nullptr);
	_children.erase(iter);

	markAsPending();
}

void uikit::Stack::clearChildren() {

	for (auto&& child : _children) {
		child->setParent(nullptr);
	}
	_children.clear();

	markAsPending();
}

void uikit::Stack::render(const ptr<ReflowCommandBuffer> cmd_) {
	for (const auto& child : *children()) {
		// Note: Drop scissor check as the stack guarantees to occupy the same space as all the children
		if (child->state().isVisible()) {
			// TODO: Check whether we need to take care of z-level ordering?
			child->render(cmd_);
		}
	}
}

void uikit::Stack::cascadeContextChange(bool invalidate_) {

	if (getEffectiveProvisioner() != None) {

		const auto& theme = getEffectiveProvisioner()->getContext();

		Opt<ref<const BoxLayoutAttributes>> layout = None;
		for (const auto& className : _staticClassList) {
			if (className.is<StringView>() && (layout = theme.getBoxLayout(className.as<StringView>())) != None) {
				break;
			}
		}

		if (layout != None) {
			auto& stored = getLayoutAttributes();

			stored.update<attr::BoxLayout::minWidth>(layout->valueOf<attr::BoxLayout::minWidth>());
			stored.update<attr::BoxLayout::width>(layout->valueOf<attr::BoxLayout::width>());
			stored.update<attr::BoxLayout::maxWidth>(layout->valueOf<attr::BoxLayout::maxWidth>());
			stored.update<attr::BoxLayout::minHeight>(layout->valueOf<attr::BoxLayout::minHeight>());
			stored.update<attr::BoxLayout::height>(layout->valueOf<attr::BoxLayout::height>());
			stored.update<attr::BoxLayout::maxHeight>(layout->valueOf<attr::BoxLayout::maxHeight>());
			stored.update<attr::BoxLayout::padding>(layout->valueOf<attr::BoxLayout::padding>());
			stored.update<attr::BoxLayout::placement>(layout->valueOf<attr::BoxLayout::placement>());
		}

	}

	/**/

	Widget::cascadeContextChange(invalidate_);
}

math::vec2 uikit::Stack::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

	const auto& layout = getLayoutAttributes();
	auto size = math::vec2 { 0.F };

	/**/

	if (layout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eAbsolute) {
		size.x = layout.valueOf<attr::BoxLayout::width>().value;
	}

	if (layout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eAbsolute) {
		size.y = layout.valueOf<attr::BoxLayout::height>().value;
	}

	/**/

	if (size.x <= 0.F || size.y <= 0.F) {

		auto computedMaxSize = math::vec2 { 0.F };
		for (const auto& child : *children()) {
			computedMaxSize = math::compMax<float>(computedMaxSize, child->getDesiredSize());
		}

		size.x = size.x > 0.F ? size.x : computedMaxSize.x;
		size.y = size.y > 0.F ? size.y : computedMaxSize.y;
	}

	/**/

	return layout::clampSizeAbs(layout, size);
}

math::vec2 uikit::Stack::computeDesiredSize(cref<ReflowPassState> passState_) const {

	const auto& layout = getLayoutAttributes();
	auto size = math::vec2 { 0.F };

	/**/

	if (layout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eAbsolute) {
		size.x = layout.valueOf<attr::BoxLayout::width>().value;

	} else if (layout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eRelative) {
		size.x = passState_.referenceSize.x * layout.valueOf<attr::BoxLayout::width>().value;
	}

	if (layout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eAbsolute) {
		size.y = layout.valueOf<attr::BoxLayout::height>().value;

	} else if (layout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eRelative) {
		size.y = passState_.referenceSize.y * layout.valueOf<attr::BoxLayout::height>().value;
	}

	/**/

	if (size.x <= 0.F || size.y <= 0.F) {

		auto computedMaxSize = math::vec2 { 0.F };
		for (const auto& child : *children()) {
			// TODO: Check whether we need to pre-compute the computed desired size of the children as we pass through the flex scaling
			computedMaxSize = math::compMax<float>(computedMaxSize, child->getDesiredSize());
		}

		size.x = size.x > 0.F ? size.x : computedMaxSize.x;
		size.y = size.y > 0.F ? size.y : computedMaxSize.y;
	}

	/**/

	return layout::clampSize(layout, passState_.layoutSize, size);
}

void uikit::Stack::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

	const auto& layout = getLayoutAttributes();

	const auto innerOffset = layout::outerToInnerOffset(layout, ctx_.localOffset);
	const auto innerSize = layout::outerToInnerSize(layout, ctx_.localSize);

	/**/

	for (const auto& child : *children()) {

		const auto childSize = math::compMin<f32>(
			math::compMax<f32>(child->layoutState().cachedPreservedSize, math::vec2 { 0.F }),
			innerSize
		);
		auto space = math::compMax(innerSize - childSize, math::vec2 { 0.F });

		switch (static_cast<u8>(layout.valueOf<attr::BoxLayout::placement>()) & (0b11 << 2)) {
			case static_cast<u8>(ReflowHorizontalPlacement::eLeft) << 2: {
				space.x = 0.F;
				break;
			}
			case static_cast<u8>(ReflowHorizontalPlacement::eCenter) << 2: {
				space.x /= 2.F;
				break;
			}
			case static_cast<u8>(ReflowHorizontalPlacement::eRight) << 2:
			default: {
				break;
			}
		}

		switch (static_cast<u8>(layout.valueOf<attr::BoxLayout::placement>()) & 0b11) {
			case static_cast<u8>(ReflowVerticalPlacement::eTop): {
				space.y = 0.F;
				break;
			}
			case static_cast<u8>(ReflowVerticalPlacement::eMiddle): {
				space.y /= 2.F;
				break;
			}
			case static_cast<u8>(ReflowVerticalPlacement::eBottom):
			default: {
				break;
			}
		}

		/**/

		const auto childState = state_.getStateOf(child);
		childState->layoutOffset = innerOffset + space;
		childState->layoutSize = childSize;
	}
}

float uikit::Stack::shrinkFactor() const noexcept {

	// TODO: Add a selector layout attribute to predefined a selected sizing behaviour (All-Max, All-Min, Fixed)
	if (
		getLayoutAttributes().valueOf<attr::BoxLayout::width>().type != ReflowUnitType::eAuto ||
		getLayoutAttributes().valueOf<attr::BoxLayout::height>().type != ReflowUnitType::eAuto
	) {
		auto computedFactor = 0.F;
		for (const auto& child : *children()) {
			computedFactor = (std::max)(computedFactor, child->shrinkFactor());
		}
		return computedFactor;
	}

	return Widget::shrinkFactor();
}

float uikit::Stack::growFactor() const noexcept {

	// TODO: Add a selector layout attribute to predefined a selected sizing behaviour (All-Max, All-Min, Fixed)
	if (
		getLayoutAttributes().valueOf<attr::BoxLayout::width>().type != ReflowUnitType::eAuto ||
		getLayoutAttributes().valueOf<attr::BoxLayout::height>().type != ReflowUnitType::eAuto
	) {
		auto computedFactor = 0.F;
		for (const auto& child : *children()) {
			computedFactor = (std::max)(computedFactor, child->growFactor());
		}
		return computedFactor;
	}

	return Widget::growFactor();
}
