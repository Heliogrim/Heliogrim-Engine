#include "BoxLayout.hpp"

#include <format>
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
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
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

	markAsPending();
}

void BoxLayout::render(const ptr<ReflowCommandBuffer> cmd_) {

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

math::vec2 BoxLayout::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

	const auto& layout = getLayoutAttributes();

	const auto inner = _children.getChild()->getDesiredSize();
	auto size = layout::innerToOuterSize(layout, inner);

	/**/

	if (layout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eAbsolute) {
		size.x = layout.valueOf<attr::BoxLayout::width>().value;
	}

	if (layout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eAbsolute) {
		size.y = layout.valueOf<attr::BoxLayout::height>().value;
	}

	/**/

	return layout::clampSizeAbs(layout, size);
}

math::vec2 BoxLayout::computeDesiredSize(cref<ReflowPassState> passState_) const {

	const auto& layout = getLayoutAttributes();

	math::vec2 desired = getDesiredSize();
	if (layout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eRelative) {
		desired.x = passState_.referenceSize.x * layout.valueOf<attr::BoxLayout::width>().value;
	}

	if (layout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eRelative) {
		desired.y = passState_.referenceSize.y * layout.valueOf<attr::BoxLayout::height>().value;
	}

	/**/

	return layout::clampSize(layout, passState_.layoutSize, desired);
}

void BoxLayout::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

	const auto& layout = getLayoutAttributes();

	const auto innerOffset = layout::outerToInnerOffset(layout, ctx_.localOffset);
	const auto innerSize = layout::outerToInnerSize(layout, ctx_.localSize);

	const auto childSize = math::compMin<float>(
		math::compMax<float>(_children.getChild()->layoutState().cachedPreservedSize, math::vec2 { 0.F }),
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
