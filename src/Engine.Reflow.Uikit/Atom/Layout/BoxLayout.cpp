#include "BoxLayout.hpp"

#include <format>
#include <Engine.Reflow/Algorithm/Flex.hpp>
#include <Engine.Reflow/Command/ReflowCommandLayer.hpp>
#include <Engine.Reflow/Layout/Style.hpp>

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

BoxLayout::BoxLayout() :
	BoxLayout(ReflowClassList { "[BoxLayout]"sv }, nullptr) {}

BoxLayout::BoxLayout(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_) :
	Widget(::hg::move(classList_), ::hg::move(parent_)),
	Atom(
		{},
		{
			.layoutAttributes = BoxLayoutAttributes {
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
	),
	_children() {}

BoxLayout::~BoxLayout() = default;

string BoxLayout::getTag() const noexcept {
	return std::format(R"(BoxPanel <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const Children> BoxLayout::children() const {
	return &_children;
}

void BoxLayout::setChild(cref<sptr<Widget>> nextChild_) {

	_children.getChild()->setParent(nullptr);

	nextChild_->setParent(shared_from_this());
	_children.setChild(nextChild_);

	markAsPending(true, true);
}

void BoxLayout::render(const ptr<ReflowCommandBuffer> cmd_) {

	const ReflowCommandLayer layer { _layoutState, *cmd_ };

	/**/

	for (const auto& child : *children()) {

		if (child->state().isVisible() && not cmd_->scissorCull(
			child->getLayoutState().layoutOffset,
			child->getLayoutState().layoutSize
		)) {
			child->render(cmd_);
		}

	}

}

PrefetchSizing BoxLayout::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {

	const auto& box = getLayoutAttributes();

	return algorithm::prefetch(
		axis_,
		{
			.mainAxis = ReflowAxis::eXAxis,
			.min = algorithm::unitAbsMin(box.valueOf<attr::BoxLayout::minWidth>(), box.valueOf<attr::BoxLayout::minHeight>()),
			.max = algorithm::unitAbsMax(box.valueOf<attr::BoxLayout::maxWidth>(), box.valueOf<attr::BoxLayout::maxHeight>()),
			.gapping = { 0.F, 0.F },
			.padding = box.valueOf<attr::BoxLayout::padding>()
		},
		children()
	);
}

PassPrefetchSizing BoxLayout::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {

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

math::fvec2 BoxLayout::computeReferenceSize(ReflowAxis axis_) const {
	const auto& padding = getLayoutAttributes().valueOf<attr::BoxLayout::padding>();

	auto result = _layoutState.computeSize;
	result.x -= (padding.x + padding.z);
	result.y -= (padding.y + padding.w);
	return result;
}

void BoxLayout::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {

	algorithm::compute(
		axis_,
		{
			.mainAxis = ReflowAxis::eXAxis,
			.size = passState_.computeSize,
			.gapping = { 0.F, 0.F },
			.padding = getLayoutAttributes().valueOf<attr::BoxLayout::padding>()
		},
		*children()
	);
}

void BoxLayout::applyLayout(ref<ReflowState> state_) {

	const auto& layout = getLayoutAttributes();

	const auto innerOffset = layout::outerToInnerOffset(layout, getLayoutState().layoutOffset);
	const auto innerSize = layout::outerToInnerSize(layout, getLayoutState().layoutSize);

	const auto childSize = math::compMin<float>(
		math::compMax<float>(_children.getChild()->getLayoutState().computeSize, math::vec2 { 0.F }),
		innerSize
	);

	/**/

	math::vec2 space = innerSize - childSize;
	space = math::compMax<float>(space, math::vec2 { 0.F });

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

	const auto childPassState = state_.getStateOf(_children.getChild());

	childPassState->layoutOffset = innerOffset + space;
	childPassState->layoutSize = childSize;
}

math::fvec2 BoxLayout::getGrowFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthGrow>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightGrow>()
	};
}

math::fvec2 BoxLayout::getShrinkFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthShrink>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightShrink>()
	};
}
