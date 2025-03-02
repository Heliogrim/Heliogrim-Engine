#include "HorizontalLayout.hpp"

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Algorithm/Flex.hpp>
#include <Engine.Reflow/Command/ReflowCommandLayer.hpp>
#include <Engine.Reflow/Layout/Style.hpp>

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

HorizontalLayout::HorizontalLayout() :
	HorizontalLayout(ReflowClassList { "[HorizontalLayout]"sv }, nullptr) {}

HorizontalLayout::HorizontalLayout(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_) :
	Widget(::hg::move(classList_), ::hg::move(parent_)),
	Atom(
		{},
		{
			.layoutAttributes = {
				.attributeSets = std::make_tuple(
					BoxLayoutAttributes {
						ReflowUnit { ReflowUnitType::eAuto, 0.F },
						ReflowUnit { ReflowUnitType::eRelative, 1.F },
						ReflowUnit { ReflowUnitType::eRelative, 1.F },
						/**/
						ReflowUnit { ReflowUnitType::eAuto, 0.F },
						ReflowUnit { ReflowUnitType::eRelative, 1.F },
						ReflowUnit { ReflowUnitType::eRelative, 1.F },
						/**/
						Padding {},
						ReflowPlacement::eMiddleCenter
					},
					FlexLayoutAttributes {
						1.F,
						1.F,
						4.F,
						4.F,
						ReflowAlignment::eCenter,
						ReflowSpacing::eStart
					}
				)
			}
		},
		{}
	),
	_children() {}

HorizontalLayout::~HorizontalLayout() = default;

string HorizontalLayout::getTag() const noexcept {
	return std::format(R"(HorizontalPanel <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const Children> HorizontalLayout::children() const {
	return &_children;
}

void HorizontalLayout::addChild(cref<sptr<Widget>> child_) {

	child_->setParent(shared_from_this());
	_children.push_back(child_);

	markAsPending();
}

void HorizontalLayout::setChild(const u32 idx_, cref<sptr<Widget>> child_) {

	if (idx_ == _children.size()) {
		addChild(child_);
	}

	if (idx_ > _children.size()) {
		return;
	}

	child_->setParent(shared_from_this());
	_children[idx_]->setParent(nullptr);
	_children[idx_] = child_;

	markAsPending();
}

void HorizontalLayout::removeChild(cref<sptr<Widget>> child_) {

	s32 found = -1L;
	for (u32 idx = 0; idx < _children.size(); ++idx) {
		if (_children[idx] == child_) {
			found = idx;
			break;
		}
	}

	/**/

	if (found < 0) {
		return;
	}

	const auto iter = _children.cbegin() + found;

	(*iter)->setParent(nullptr);
	_children.erase(_children.cbegin() + found);

	markAsPending();
}

void HorizontalLayout::clearChildren() {

	for (const auto& child : _children) {
		child->setParent(nullptr);
	}

	_children.clear();

	markAsPending();
}

void HorizontalLayout::render(const ptr<ReflowCommandBuffer> cmd_) {

	const ReflowCommandLayer layer { _layoutState, *cmd_ };

	/**/

	for (const auto& child : *children()) {

		if (child->state().isVisible() && not cmd_->scissorCull(
			child->layoutState().layoutOffset,
			child->layoutState().layoutSize
		)) {
			child->render(cmd_);
		}

	}
}

math::vec2 HorizontalLayout::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

	const auto& layout = getLayoutAttributes();
	const auto& boxLayout = std::get<0>(layout.attributeSets);
	const auto& flexLayout = std::get<1>(layout.attributeSets);

	math::vec2 childAggregate;
	math::vec2 childMax;

	for (const auto& child : *children()) {

		if (child->position() == ReflowPosition::eAbsolute) {
			continue;
		}

		const auto childState = state_.getStateOf(child);

		childAggregate += child->getDesiredSize();
		childMax = math::compMax<float>(childMax, child->getDesiredSize());
	}

	const auto gapping = math::vec2 {
		flexLayout.valueOf<attr::FlexLayout::colGap>(),
		flexLayout.valueOf<attr::FlexLayout::rowGap>()
	} * static_cast<float>(MAX(children()->size(), 1) - 1);

	/**/

	const math::vec2 innerSize = math::vec2 { childAggregate.x + gapping.x, childMax.y };
	math::vec2 size = layout::innerToOuterSize(boxLayout, innerSize);

	/**/

	if (boxLayout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eAbsolute) {
		size.x = boxLayout.valueOf<attr::BoxLayout::width>().value;
	}
	if (boxLayout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eAbsolute) {
		size.y = boxLayout.valueOf<attr::BoxLayout::height>().value;
	}

	/**/

	return layout::clampSizeAbs(boxLayout, size);
}

math::vec2 HorizontalLayout::computeDesiredSize(cref<ReflowPassState> passState_) const {

	const auto& layout = getLayoutAttributes();
	const auto& boxLayout = std::get<0>(layout.attributeSets);

	/**/

	math::vec2 desired { getDesiredSize() };
	if (boxLayout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eRelative) {
		desired.x = passState_.referenceSize.x * boxLayout.valueOf<attr::BoxLayout::width>().value;
	}
	if (boxLayout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eRelative) {
		desired.y = passState_.referenceSize.y * boxLayout.valueOf<attr::BoxLayout::height>().value;
	}

	return layout::clampSize(boxLayout, passState_.layoutSize, desired);
}

void HorizontalLayout::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

	const auto& layout = getLayoutAttributes();
	const auto& boxLayout = std::get<0>(layout.attributeSets);
	const auto& flexLayout = std::get<1>(layout.attributeSets);

	/**/

	const auto innerSize = layout::outerToInnerSize(boxLayout, ctx_.localSize);

	algorithm::FlexState flexState {};
	flexState.box.preservedSize = innerSize;
	flexState.box.maxSize = innerSize;
	flexState.box.orientation = algorithm::FlexLineOrientation::eHorizontal;
	flexState.box.justify = flexLayout.valueOf<attr::FlexLayout::justify>();
	flexState.box.align = flexLayout.valueOf<attr::FlexLayout::align>();
	flexState.box.gap = math::vec2 { flexLayout.valueOf<attr::FlexLayout::colGap>(), flexLayout.valueOf<attr::FlexLayout::rowGap>() };
	flexState.box.wrap = false;

	/**/

	algorithm::solve(flexState, state_, children());

	/**/

	const auto minFlexBound = /*ctx_.localOffset*/math::vec2 { 0.F };
	const auto maxFlexBound = /*ctx_.localOffset + ctx_.localSize*/innerSize;

	const auto offset = layout::outerToInnerOffset(boxLayout, ctx_.localOffset);

	/**/

	for (const auto& flexLine : flexState.lines) {
		for (const auto& flexItem : flexLine.items) {

			const auto dummy = flexItem.widget.lock();
			const auto widgetState = state_.getStateOf(std::static_pointer_cast<Widget, void>(dummy));

			widgetState->layoutOffset = flexItem.offset + offset;
			widgetState->layoutSize = flexItem.flexSize;

			// TODO: Check how we should work with co-axis
			// Currently Flex-Solving will guarantee main-axis constraint (as long as possible), but co-axis will break
			// We might try to hard-limit contextual constraints

			math::vec2 minDiff;
			math::vec2 maxDiff;

			const auto minCorner = flexItem.offset;
			const auto maxCorner = flexItem.offset + flexItem.flexSize;

			if (maxCorner.x > maxFlexBound.x || maxCorner.y > maxFlexBound.y) {
				maxDiff = maxCorner - maxFlexBound;
				maxDiff = math::compMax<float>(maxDiff, math::vec2 { 0.F });
			}

			if (minCorner.x > minFlexBound.x || minCorner.y > minFlexBound.y) {
				minDiff = minFlexBound - minCorner;
				minDiff = math::compMax<float>(minDiff, math::vec2 { 0.F });
			}

			widgetState->layoutOffset += minDiff;
			widgetState->layoutSize -= maxDiff;
		}
	}

	/**/

	// Warning: Hotfix
	for (const auto& child : _children) {
		if (child->position() == ReflowPosition::eAbsolute) {

			const auto widgetState = state_.getStateOf(child);
			widgetState->layoutOffset = ctx_.localOffset;
			widgetState->layoutSize = widgetState->cachedPreservedSize;
		}
	}
}

float HorizontalLayout::shrinkFactor() const noexcept {
	const auto& layout = getLayoutAttributes();
	const auto& flexLayout = std::get<1>(layout.attributeSets);

	return flexLayout.valueOf<attr::FlexLayout::shrink>();
}

float HorizontalLayout::growFactor() const noexcept {
	const auto& layout = getLayoutAttributes();
	const auto& flexLayout = std::get<1>(layout.attributeSets);

	return flexLayout.valueOf<attr::FlexLayout::grow>();
}
