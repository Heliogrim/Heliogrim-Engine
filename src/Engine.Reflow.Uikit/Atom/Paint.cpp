#include "Paint.hpp"

#include <format>
#include <Engine.Reflow/Algorithm/Flex.hpp>

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

Paint::Paint() :
	Paint(ReflowClassList { "[Paint]"sv }, nullptr) {}

Paint::Paint(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_) :
	LeafWidget(::hg::move(classList_), ::hg::move(parent_)),
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
	) {}

Paint::~Paint() = default;

string Paint::getTag() const noexcept {
	return std::format(R"(Paint <{:#x}>)", reinterpret_cast<u64>(this));
}

void Paint::render(const ptr<ReflowCommandBuffer> cmd_) {

	const auto padding = getLayoutAttributes().valueOf<attr::BoxLayout::padding>();
	const auto borderRadius = getStyleAttributes().valueOf<attr::PaintStyle::borderRadius>();
	auto offset = _layoutState.layoutOffset;
	auto size = _layoutState.layoutSize;

	offset.x += padding.x;
	offset.y += padding.y;

	size.x -= (padding.x + padding.z);
	size.y -= (padding.y + padding.w);

	/**/

	if (borderRadius.allZero()) {
		cmd_->drawRect(offset, offset + size, getStyleAttributes().valueOf<attr::PaintStyle::tint>());
		return;
	}

	/**/

	const auto innerMin = offset + math::fvec2 {
		std::max(borderRadius.x, borderRadius.w), std::max(borderRadius.x, borderRadius.y)
	};
	const auto innerMax = offset + size - math::fvec2 {
		std::max(borderRadius.z, borderRadius.y), std::max(borderRadius.z, borderRadius.w)
	};

	cmd_->drawRect(innerMin, innerMax, getStyleAttributes().valueOf<attr::PaintStyle::tint>());

	// Top-Left Corner
	cmd_->drawArc(
		offset + math::fvec2 { borderRadius.x },
		borderRadius.x,
		1.F * math::pi,
		1.5F * math::pi,
		getStyleAttributes().valueOf<attr::PaintStyle::tint>()
	);

	cmd_->drawRect(
		math::fvec2 { offset.x + borderRadius.x, offset.y },
		math::fvec2 { offset.x + size.x - borderRadius.y, offset.y + std::max(borderRadius.x, borderRadius.y) },
		getStyleAttributes().valueOf<attr::PaintStyle::tint>()
	);

	// Top-Right Corner
	cmd_->drawArc(
		offset + math::fvec2 { size.x - borderRadius.y, borderRadius.y },
		borderRadius.x,
		1.5F * math::pi,
		2.F * math::pi,
		getStyleAttributes().valueOf<attr::PaintStyle::tint>()
	);

	cmd_->drawRect(
		math::fvec2 { offset.x + size.x - std::max(borderRadius.y, borderRadius.z), offset.y + borderRadius.y },
		math::fvec2 { offset.x + size.x, offset.y + size.y - borderRadius.z },
		getStyleAttributes().valueOf<attr::PaintStyle::tint>()
	);

	// Bottom-Right Corner
	cmd_->drawArc(
		offset + size - math::fvec2 { borderRadius.z },
		borderRadius.x,
		0.F * math::pi,
		0.5F * math::pi,
		getStyleAttributes().valueOf<attr::PaintStyle::tint>()
	);

	cmd_->drawRect(
		math::fvec2 { offset.x + borderRadius.w, offset.y + size.y - std::max(borderRadius.w, borderRadius.z) },
		math::fvec2 { offset.x + size.x - borderRadius.z, offset.y + size.y },
		getStyleAttributes().valueOf<attr::PaintStyle::tint>()
	);

	// Bottom-Left Corner
	cmd_->drawArc(
		offset + math::fvec2 { borderRadius.w, size.y - borderRadius.w },
		borderRadius.x,
		0.5F * math::pi,
		1.F * math::pi,
		getStyleAttributes().valueOf<attr::PaintStyle::tint>()
	);

	cmd_->drawRect(
		math::fvec2 { offset.x, offset.y + borderRadius.x },
		math::fvec2 { offset.x + std::max(borderRadius.x, borderRadius.w), offset.y + size.y - borderRadius.w },
		getStyleAttributes().valueOf<attr::PaintStyle::tint>()
	);

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
			getStyleAttributes().update<attr::PaintStyle::tint>(
				paint->valueOf<attr::PaintStyle::tint>()
			);
			getStyleAttributes().update<attr::PaintStyle::borderRadius>(
				paint->valueOf<attr::PaintStyle::borderRadius>()
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
