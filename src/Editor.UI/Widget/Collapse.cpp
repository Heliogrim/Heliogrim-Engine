#include "Collapse.hpp"

#include <format>
#include <Engine.Reflow/Algorithm/Flex.hpp>
#include <Engine.Reflow/Layout/Style.hpp>

using namespace hg::engine::reflow;
using namespace hg::editor::ui;
using namespace hg;

Collapse::Collapse() :
	Collapse(ReflowClassList {}, nullptr) {}

Collapse::Collapse(mref<engine::reflow::ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_) :
	Widget(::hg::move(classList_), ::hg::move(parent_)),
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
				Padding {},
				ReflowPlacement::eMiddleCenter
			}
		},
		{}
	),
	_collapsed(),
	_toggleHandle(~ReflowEventEmitter::handle_type {}),
	_children() {}

Collapse::~Collapse() = default;

string Collapse::getTag() const noexcept {
	return std::format(R"(Collapse <{:#x}>)", reinterpret_cast<u64>(this));
}

void Collapse::collapse() {
	_collapsed = true;
	markAsPending();

	const auto body = _children.getChild<1uL>();
	if (body != nullptr && body != NullWidget::instance()) {
		body->state().unset(WidgetStateFlagBits::eVisible);
		body->markAsPending();
	}
}

void Collapse::expand() {
	_collapsed = false;
	markAsPending();

	const auto body = _children.getChild<1uL>();
	if (body != nullptr && body != NullWidget::instance()) {
		body->state().set(WidgetStateFlagBits::eVisible);
		body->markAsPending();
	}
}

const ptr<const FixedChildren<2>> Collapse::children() const noexcept {
	return &_children;
}

void Collapse::setHeader(mref<SharedPtr<Widget>> header_) {

	auto prevToggleHandle = _toggleHandle;
	if (header_ != nullptr && header_ != NullWidget::instance()) {
		_toggleHandle = header_->onClick(
			[&]([[maybe_unused]] const auto& event_) {
				if (_collapsed) {
					this->expand();
				} else {
					this->collapse();
				}
				return EventResponse::eConsumed;
			}
		);
	}

	header_->setParent(shared_from_this());
	auto prev = _children.setChild<0>(::hg::move(header_));

	if (prev != nullptr && prev != NullWidget::instance()) {
		prev->setParent(nullptr);
		prev->dropOnClick(prevToggleHandle);
	}

	markAsPending();
}

void Collapse::setContent(mref<SharedPtr<Widget>> content_) {
	content_->setParent(shared_from_this());
	auto prev = _children.setChild<1>(::hg::move(content_));
	prev->setParent(nullptr);

	if (_collapsed)
		collapse();
	else
		expand();
}

void Collapse::render(const ptr<engine::reflow::ReflowCommandBuffer> cmd_) {

	math::fExtent2D scissor = math::fExtent2D {
		_layoutState.layoutSize.x, _layoutState.layoutSize.y,
		_layoutState.layoutOffset.x, _layoutState.layoutOffset.y
	};

	cmd_->pushIntersectScissor(scissor);

	/**/

	for (const auto& child : _children) {

		if (child->state().isVisible() && not cmd_->scissorCull(
			child->getLayoutState().layoutOffset,
			child->getLayoutState().layoutSize
		)) {
			child->render(cmd_);
		}

	}

	/**/

	cmd_->popScissor();
}

PrefetchSizing Collapse::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {

	const auto& box = getLayoutAttributes();
	auto pb = algorithm::PrefetchBox {
		.mainAxis = ReflowAxis::eYAxis,
		.min = algorithm::unitAbsMin(box.valueOf<attr::BoxLayout::minWidth>(), box.valueOf<attr::BoxLayout::minHeight>()),
		.max = algorithm::unitAbsMax(box.valueOf<attr::BoxLayout::maxWidth>(), box.valueOf<attr::BoxLayout::maxHeight>()),
		.gapping = {},
		.padding = box.valueOf<attr::BoxLayout::padding>()
	};

	auto min = math::fvec2 {};
	auto size = math::fvec2 {};

	const auto& header = _children.getChild<0>();
	if (header != nullptr && header != NullWidget::instance()) {
		min.x = std::max(min.x, header->getLayoutState().prefetchMinSize.x);
		min.y += header->getLayoutState().prefetchMinSize.y;
		size.x = std::max(size.x, header->getLayoutState().prefetchSize.x);
		size.y += header->getLayoutState().prefetchSize.y;
	}

	const auto& content = _children.getChild<1>();
	if (not _collapsed && content != nullptr && content != NullWidget::instance()) {
		min.x = std::max(min.x, content->getLayoutState().prefetchMinSize.x);
		min.y += content->getLayoutState().prefetchMinSize.y;
		size.x = std::max(size.x, content->getLayoutState().prefetchSize.x);
		size.y += content->getLayoutState().prefetchSize.y;
	}

	min += math::fvec2 { pb.padding.x + pb.padding.z, pb.padding.y + pb.padding.w };
	size += math::fvec2 { pb.padding.x + pb.padding.z, pb.padding.y + pb.padding.w };

	return {
		math::compClamp(pb.min, min, pb.max),
		math::compClamp(pb.min, size, pb.max)
	};
}

engine::reflow::PassPrefetchSizing Collapse::passPrefetchSizing(
	engine::reflow::ReflowAxis axis_,
	ref<const engine::reflow::ReflowPassState> passState_
) const {

	const auto& box = getLayoutAttributes<>();
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

math::fvec2 Collapse::computeReferenceSize(engine::reflow::ReflowAxis axis_) const {

	const auto& box = getLayoutAttributes();
	const auto& padding = box.valueOf<attr::BoxLayout::padding>();

	auto result = _layoutState.computeSize;
	result.x -= (padding.x + padding.z);
	result.y -= (padding.y + padding.w);
	return result;
}

void Collapse::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {

	const auto& box = getLayoutAttributes();

	algorithm::compute(
		axis_,
		{
			.mainAxis = ReflowAxis::eYAxis,
			.size = passState_.computeSize,
			.gapping = {},
			.padding = box.valueOf<attr::BoxLayout::padding>()
		},
		_collapsed ? std::span { _children.begin(), 1uLL } : std::span { _children }
	);
}

void Collapse::applyLayout(ref<ReflowState> state_) {

	const auto& box = getLayoutAttributes();

	algorithm::layout(
		{
			.mainAxis = ReflowAxis::eYAxis,
			.anchor = getLayoutState().layoutOffset,
			.span = getLayoutState().layoutSize,
			.gapping = {},
			.padding = box.valueOf<attr::BoxLayout::padding>(),
			.align = ReflowAlignment::eStart,
			.justify = ReflowSpacing::eStart
		},
		children()
	);
}

math::fvec2 Collapse::getGrowFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthShrink>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightShrink>()
	};
}

math::fvec2 Collapse::getShrinkFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthGrow>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightGrow>()
	};
}
