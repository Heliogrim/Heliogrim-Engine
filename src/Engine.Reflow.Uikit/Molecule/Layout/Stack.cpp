#include "Stack.hpp"

#include <format>
#include <Engine.Reflow/Algorithm/Flex.hpp>
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
				0.F, 1.F,
				/**/
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				0.F, 1.F,
				/**/
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
			stored.update<attr::BoxLayout::maxWidth>(layout->valueOf<attr::BoxLayout::maxWidth>());
			stored.update<attr::BoxLayout::widthGrow>(layout->valueOf<attr::BoxLayout::widthGrow>());
			stored.update<attr::BoxLayout::widthShrink>(layout->valueOf<attr::BoxLayout::widthShrink>());
			stored.update<attr::BoxLayout::minHeight>(layout->valueOf<attr::BoxLayout::minHeight>());
			stored.update<attr::BoxLayout::maxHeight>(layout->valueOf<attr::BoxLayout::maxHeight>());
			stored.update<attr::BoxLayout::heightGrow>(layout->valueOf<attr::BoxLayout::heightGrow>());
			stored.update<attr::BoxLayout::heightShrink>(layout->valueOf<attr::BoxLayout::heightShrink>());
			stored.update<attr::BoxLayout::padding>(layout->valueOf<attr::BoxLayout::padding>());
			stored.update<attr::BoxLayout::placement>(layout->valueOf<attr::BoxLayout::placement>());
		}

	}

	/**/

	Widget::cascadeContextChange(invalidate_);
}

PrefetchSizing uikit::Stack::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {

	const auto& layout = getLayoutAttributes();

	const auto minSize = algorithm::unitAbsMin(layout.valueOf<attr::BoxLayout::minWidth>(), layout.valueOf<attr::BoxLayout::minHeight>());
	const auto maxSize = algorithm::unitAbsMax(layout.valueOf<attr::BoxLayout::maxWidth>(), layout.valueOf<attr::BoxLayout::maxHeight>());

	/**/

	auto innerMin = math::vec2 { 0.F };
	auto innerSize = math::vec2 { 0.F };
	for (const auto& child : *children()) {
		innerMin = math::compMax<float>(innerMin, child->getLayoutState().prefetchMinSize);
		innerSize = math::compMax<float>(innerSize, child->getLayoutState().prefetchSize);
	}

	const auto& padding = layout.valueOf<attr::BoxLayout::padding>();
	innerMin += math::fvec2 { padding.x + padding.z, padding.y + padding.w };
	innerSize += math::fvec2 { padding.x + padding.z, padding.y + padding.w };

	/**/

	return {
		math::compClamp(minSize, innerMin, maxSize),
		math::compClamp(minSize, innerSize, maxSize)
	};
}

PassPrefetchSizing uikit::Stack::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {

	const auto& box = getLayoutAttributes();
	return {
		algorithm::nextUnit(
			box.valueOf<attr::BoxLayout::minWidth>(),
			box.valueOf<attr::BoxLayout::minHeight>(),
			passState_.referenceSize,
			passState_.prefetchMinSize
		),
		math::compMax(
			algorithm::nextUnit(
				box.valueOf<attr::BoxLayout::minWidth>(),
				box.valueOf<attr::BoxLayout::minHeight>(),
				passState_.referenceSize,
				passState_.prefetchSize
			),
			passState_.prefetchSize
		),
		algorithm::nextUnit(
			box.valueOf<attr::BoxLayout::maxWidth>(),
			box.valueOf<attr::BoxLayout::maxHeight>(),
			passState_.referenceSize,
			algorithm::unitAbsMax(
				box.valueOf<attr::BoxLayout::maxWidth>(),
				box.valueOf<attr::BoxLayout::maxHeight>()
			)
		)
	};
}

math::fvec2 uikit::Stack::computeReferenceSize(ReflowAxis axis_) const {

	const auto& padding = getLayoutAttributes().valueOf<attr::BoxLayout::padding>();

	auto result = _layoutState.computeSize;
	result.x -= (padding.x + padding.z);
	result.y -= (padding.y + padding.w);
	return result;
}

void uikit::Stack::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {

	// TODO: Padding
	const auto& layout = getLayoutAttributes();
	const auto& padding = layout.valueOf<attr::BoxLayout::padding>();

	const auto space = passState_.computeSize - math::fvec2 { padding.x + padding.z, padding.y + padding.w };

	for (const auto& child : *children()) {
		if (axis_ == ReflowAxis::eXAxis) {
			const auto perChildLimit = child->getGrowFactor().x > 0.F ?
				child->getLayoutState().prefetchMaxSize.x :
				child->getLayoutState().prefetchSize.x;
			child->getLayoutState().computeSize.x = std::min(space.x, perChildLimit);

		} else {
			const auto perChildLimit = child->getGrowFactor().y > 0.F ?
				child->getLayoutState().prefetchMaxSize.y :
				child->getLayoutState().prefetchSize.y;
			child->getLayoutState().computeSize.y = std::min(space.y, perChildLimit);
		}
	}
}

void uikit::Stack::applyLayout(ref<ReflowState> state_) {

	const auto& layout = getLayoutAttributes();

	const auto innerOffset = layout::outerToInnerOffset(layout, getLayoutState().layoutOffset);
	const auto innerSize = layout::outerToInnerSize(layout, getLayoutState().layoutSize);

	/**/

	for (const auto& child : *children()) {

		const auto childSize = math::compMin<f32>(
			math::compMax<f32>(child->getLayoutState().computeSize, math::vec2 { 0.F }),
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

math::fvec2 uikit::Stack::getShrinkFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthShrink>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightShrink>()
	};
}

math::fvec2 uikit::Stack::getGrowFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthGrow>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightGrow>()
	};
}
