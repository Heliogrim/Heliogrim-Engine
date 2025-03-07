#include "Paint.hpp"

#include <format>
#include <Engine.Reflow/Algorithm/Flex.hpp>

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

Paint::Paint() :
	Paint(ReflowClassList { "[Paint]"sv }, nullptr) {}

Paint::Paint(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_) :
	Widget(::hg::move(classList_), ::hg::move(parent_)),
	Atom(
		{},
		{
			BoxLayoutAttributes {
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				1.F, 1.F,
				/**/
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				1.F, 1.F,
				/**/
				Padding {},
				ReflowPlacement::eMiddleCenter
			}
		},
		{ PaintStyleAttributes {} }
	),
	_children() {}

Paint::~Paint() = default;

string Paint::getTag() const noexcept {
	return std::format(R"(Paint <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const NullChildren> Paint::children() const {
	return &_children;
}

void Paint::render(const ptr<ReflowCommandBuffer> cmd_) {

	const auto padding = getLayoutAttributes().valueOf<attr::BoxLayout::padding>();
	auto offset = _layoutState.layoutOffset;
	auto size = _layoutState.layoutSize;

	offset.x += padding.x;
	offset.y += padding.y;

	size.x -= (padding.x + padding.z);
	size.y -= (padding.y + padding.w);

	cmd_->drawRect(offset, offset + size, getStyleAttributes().valueOf<attr::PaintStyleAttributes::tint>());
}

void Paint::cascadeContextChange(bool invalidate_) {

	if (getEffectiveProvisioner() != None) {

		const auto& theme = getEffectiveProvisioner()->getContext();

		Opt<ref<const PaintStyleAttributes>> paint = None;
		for (const auto& className : _staticClassList) {
			if (className.is<StringView>() && (paint = theme.getPaintStyle(className.as<StringView>())) != None) {
				break;
			}
		}

		if (paint != None) {
			getStyleAttributes().update<attr::PaintStyleAttributes::tint>(
				paint->valueOf<attr::PaintStyleAttributes::tint>()
			);
		}

	}

	/**/

	Widget::cascadeContextChange(invalidate_);
}

PrefetchSizing Paint::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {

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

PassPrefetchSizing Paint::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {

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

void Paint::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {}

void Paint::applyLayout(ref<ReflowState> state_) {}

math::fvec2 Paint::getShrinkFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthShrink>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightShrink>()
	};
}

math::fvec2 Paint::getGrowFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthGrow>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightGrow>()
	};
}
