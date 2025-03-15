#include "VerticalLayout.hpp"

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Algorithm/Flex.hpp>
#include <Engine.Reflow/Command/ReflowCommandLayer.hpp>
#include <Engine.Reflow/Layout/Style.hpp>

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

VerticalLayout::VerticalLayout():
	VerticalLayout(ReflowClassList { "[VerticalLayout]"sv }, nullptr) {}

VerticalLayout::VerticalLayout(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_) :
	Widget(::hg::move(classList_), ::hg::move(parent_)),
	Atom(
		{},
		{
			.layoutAttributes = {
				.attributeSets = std::make_tuple(
					BoxLayoutAttributes {
						ReflowUnit { ReflowUnitType::eAuto, 0.F },
						ReflowUnit { ReflowUnitType::eAuto, 0.F },
						0.F, 1.F,
						/**/
						ReflowUnit { ReflowUnitType::eAuto, 0.F },
						ReflowUnit { ReflowUnitType::eAuto, 0.F },
						0.F, 1.F,
						/**/
						Padding {},
						ReflowPlacement::eMiddleCenter
					},
					FlexLayoutAttributes {
						4.F,
						4.F,
						ReflowAlignment::eStart,
						ReflowSpacing::eStart
					}
				)
			}
		},
		{}
	),
	_children() {}

VerticalLayout::~VerticalLayout() = default;

string VerticalLayout::getTag() const noexcept {
	return std::format(R"(VerticalPanel <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const Children> VerticalLayout::children() const {
	return &_children;
}

void VerticalLayout::addChild(cref<sptr<Widget>> child_) {

	child_->setParent(shared_from_this());
	_children.push_back(child_);

	markAsPending();
}

void VerticalLayout::setChild(const u32 idx_, cref<sptr<Widget>> child_) {

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

void VerticalLayout::removeChild(cref<sptr<Widget>> child_) {

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

void VerticalLayout::clearChildren() {

	for (const auto& child : _children) {
		child->setParent(nullptr);
	}

	_children.clear();

	markAsPending();
}

void VerticalLayout::render(const ptr<ReflowCommandBuffer> cmd_) {

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

PrefetchSizing VerticalLayout::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {

	const auto& layout = getLayoutAttributes();
	const auto& box = std::get<0>(layout.attributeSets);
	const auto& flex = std::get<1>(layout.attributeSets);

	return algorithm::prefetch(
		axis_,
		{
			.mainAxis = ReflowAxis::eYAxis,
			.min = algorithm::unitAbsMin(box.valueOf<attr::BoxLayout::minWidth>(), box.valueOf<attr::BoxLayout::minHeight>()),
			.max = algorithm::unitAbsMax(box.valueOf<attr::BoxLayout::maxWidth>(), box.valueOf<attr::BoxLayout::maxHeight>()),
			.gapping = { flex.valueOf<attr::FlexLayout::colGap>(), flex.valueOf<attr::FlexLayout::rowGap>() },
			.padding = box.valueOf<attr::BoxLayout::padding>()
		},
		children()
	);
}

PassPrefetchSizing VerticalLayout::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {

	const auto& box = std::get<0>(getLayoutAttributes().attributeSets);
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

math::fvec2 VerticalLayout::computeReferenceSize(ReflowAxis axis_) const {

	const auto& box = std::get<0>(getLayoutAttributes().attributeSets);
	const auto& flex = std::get<1>(getLayoutAttributes().attributeSets);

	const auto& padding = box.valueOf<attr::BoxLayout::padding>();

	auto result = _layoutState.computeSize;
	const auto gapCount = _children.empty() ? 0uLL : _children.size() - 1uLL;
	result.x -= (padding.x + padding.z);
	result.y -= (padding.y + padding.w + flex.valueOf<attr::FlexLayout::rowGap>() * static_cast<f32>(gapCount));
	return result;
}

void VerticalLayout::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {

	const auto& box = std::get<0>(getLayoutAttributes().attributeSets);
	const auto& flex = std::get<1>(getLayoutAttributes().attributeSets);

	algorithm::compute(
		axis_,
		{
			.mainAxis = ReflowAxis::eYAxis,
			.size = passState_.computeSize,
			.gapping = { flex.valueOf<attr::FlexLayout::colGap>(), flex.valueOf<attr::FlexLayout::rowGap>() },
			.padding = box.valueOf<attr::BoxLayout::padding>()
		},
		children()
	);
}

void VerticalLayout::applyLayout(ref<ReflowState> state_) {

	const auto& layout = getLayoutAttributes();
	const auto& box = std::get<0>(layout.attributeSets);
	const auto& flex = std::get<1>(layout.attributeSets);

	/**/

	algorithm::layout(
		{
			.mainAxis = ReflowAxis::eYAxis,
			.anchor = getLayoutState().layoutOffset,
			.span = getLayoutState().layoutSize,
			.gapping = { flex.valueOf<attr::FlexLayout::colGap>(), flex.valueOf<attr::FlexLayout::rowGap>() },
			.padding = box.valueOf<attr::BoxLayout::padding>(),
			.align = flex.valueOf<attr::FlexLayout::align>(),
			.justify = flex.valueOf<attr::FlexLayout::justify>()
		},
		children()
	);

	/**/

	// Warning: Hotfix
	for (const auto& child : _children) {
		if (child->position() == ReflowPosition::eAbsolute) {

			const auto widgetState = state_.getStateOf(child);
			widgetState->layoutOffset = getLayoutState().layoutOffset;
			widgetState->layoutSize = widgetState->computeSize;
		}
	}
}

math::fvec2 VerticalLayout::getShrinkFactor() const noexcept {
	const auto& box = std::get<0>(getLayoutAttributes().attributeSets);

	return {
		box.valueOf<attr::BoxLayout::widthShrink>(),
		box.valueOf<attr::BoxLayout::heightShrink>()
	};
};

math::fvec2 VerticalLayout::getGrowFactor() const noexcept {
	const auto& box = std::get<0>(getLayoutAttributes().attributeSets);

	return {
		box.valueOf<attr::BoxLayout::widthGrow>(),
		box.valueOf<attr::BoxLayout::heightGrow>()
	};
}
