#include "HorizontalLayout.hpp"

import Engine.Reflow;
import std;

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
				std::make_tuple(
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
	_children() {
	bindAttributeOwner(this);
}

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

	markAsPending(true, true);
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

	markAsPending(true, true);
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

	markAsPending(true, true);
}

void HorizontalLayout::clearChildren() {

	for (const auto& child : _children) {
		child->setParent(nullptr);
	}

	_children.clear();

	markAsPending(true, true);
}

void HorizontalLayout::render(const ptr<ReflowCommandBuffer> cmd_) {

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

PrefetchSizing HorizontalLayout::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {

	return algorithm::prefetch(
		axis_,
		{
			.mainAxis = ReflowAxis::eXAxis,
			.min = algorithm::unitAbsMin(this->get<attr::BoxLayout::minWidth>(), this->get<attr::BoxLayout::minHeight>()),
			.max = algorithm::unitAbsMax(this->get<attr::BoxLayout::maxWidth>(), this->get<attr::BoxLayout::maxHeight>()),
			.gapping = { this->get<attr::FlexLayout::colGap>(), this->get<attr::FlexLayout::rowGap>() },
			.padding = this->get<attr::BoxLayout::padding>()
		},
		children()
	);
}

PassPrefetchSizing HorizontalLayout::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {

	return {
		algorithm::nextUnit(
			this->get<attr::BoxLayout::minWidth>(),
			this->get<attr::BoxLayout::minHeight>(),
			passState_.referenceSize,
			passState_.prefetchMinSize
		),
		math::compMax(
			algorithm::nextUnit(
				this->get<attr::BoxLayout::minWidth>(),
				this->get<attr::BoxLayout::minHeight>(),
				passState_.referenceSize,
				passState_.prefetchSize
			),
			passState_.prefetchSize
		),
		algorithm::nextUnit(
			this->get<attr::BoxLayout::maxWidth>(),
			this->get<attr::BoxLayout::maxHeight>(),
			passState_.referenceSize,
			algorithm::unitAbsMax(
				this->get<attr::BoxLayout::maxWidth>(),
				this->get<attr::BoxLayout::maxHeight>()
			)
		)
	};
}

math::fvec2 HorizontalLayout::computeReferenceSize(ReflowAxis axis_) const {

	const auto& padding = this->get<attr::BoxLayout::padding>();

	auto result = _layoutState.computeSize;
	const auto gapCount = _children.empty() ? 0uLL : _children.size() - 1uLL;
	result.x -= (padding.x + padding.z + this->get<attr::FlexLayout::colGap>() * static_cast<f32>(gapCount));
	result.y -= (padding.y + padding.w);
	return result;
}

void HorizontalLayout::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {

	algorithm::compute(
		axis_,
		{
			.mainAxis = ReflowAxis::eXAxis,
			.size = passState_.computeSize,
			.gapping = { this->get<attr::FlexLayout::colGap>(), this->get<attr::FlexLayout::rowGap>() },
			.padding = this->get<attr::BoxLayout::padding>()
		},
		*children()
	);
}

void HorizontalLayout::applyLayout(ref<ReflowState> state_) {

	/**/

	algorithm::layout(
		{
			.mainAxis = ReflowAxis::eXAxis,
			.anchor = getLayoutState().layoutOffset,
			.span = getLayoutState().layoutSize,
			.gapping = { this->get<attr::FlexLayout::colGap>(), this->get<attr::FlexLayout::rowGap>() },
			.padding = this->get<attr::BoxLayout::padding>(),
			.align = this->get<attr::FlexLayout::align>(),
			.justify = this->get<attr::FlexLayout::justify>()
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

math::fvec2 HorizontalLayout::getShrinkFactor() const noexcept {

	return {
		this->get<attr::BoxLayout::widthShrink>(),
		this->get<attr::BoxLayout::heightShrink>()
	};
};

math::fvec2 HorizontalLayout::getGrowFactor() const noexcept {

	return {
		this->get<attr::BoxLayout::widthGrow>(),
		this->get<attr::BoxLayout::heightGrow>()
	};
}
