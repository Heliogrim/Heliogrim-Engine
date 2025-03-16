#include "UniformGridLayout.hpp"

#include <Engine.Logging/Logger.hpp>
#include <Engine.Reflow/Algorithm/Flex.hpp>
#include <Engine.Reflow/Layout/Style.hpp>

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

UniformGridLayout::UniformGridLayout() :
	UniformGridLayout(
		ReflowClassList {
			/* "[UniformGridLayout]"sv */
		},
		nullptr
	) {}

UniformGridLayout::UniformGridLayout(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_) :
	GridLayoutBase(::hg::move(classList_), ::hg::move(parent_)),
	Atom(
		{},
		{
			.layoutAttributes = std::make_tuple(
				BoxLayoutAttributes {
					ReflowUnit { ReflowUnitType::eAuto, 0.F },
					ReflowUnit { ReflowUnitType::eAuto, 0.F },
					0.F, 1.F,
					/**/
					ReflowUnit { ReflowUnitType::eAuto, 0.F },
					ReflowUnit { ReflowUnitType::eAuto, 0.F },
					0.F, 1.F,
					/**/
					Padding { 4.F },
					ReflowPlacement::eMiddleCenter
				},
				FlexLayoutAttributes {
					4.F,
					4.F,
					ReflowAlignment::eStart,
					ReflowSpacing::eStart
				},
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
			)
		},
		{}
	) {}

UniformGridLayout::~UniformGridLayout() = default;

string UniformGridLayout::getTag() const noexcept {
	return std::format(R"(UniformGridPanel <{:#x}>)", reinterpret_cast<u64>(this));
}

PrefetchSizing UniformGridLayout::prefetchSlotSizingX() const {

	const auto& slot = std::get<2>(getLayoutAttributes().attributeSets);

	// TODO: Check for auto-sizing requiring size measure of the slot child element

	return algorithm::prefetch(
		ReflowAxis::eXAxis,
		{
			.mainAxis = ReflowAxis::eXAxis,
			.min = algorithm::unitAbsMin(slot.valueOf<attr::BoxLayout::minWidth>(), slot.valueOf<attr::BoxLayout::minHeight>()),
			.max = algorithm::unitAbsMax(slot.valueOf<attr::BoxLayout::maxWidth>(), slot.valueOf<attr::BoxLayout::maxHeight>()),
			.gapping = { 0.F, 0.F },
			.padding = slot.valueOf<attr::BoxLayout::padding>()
		},
		get_null_children()
	);
}

PrefetchSizing UniformGridLayout::prefetchSlotSizingY() const {

	const auto& box = std::get<0>(getLayoutAttributes().attributeSets);
	const auto& flex = std::get<1>(getLayoutAttributes().attributeSets);

	const auto& padding = box.valueOf<attr::BoxLayout::padding>();
	const auto gapping = math::fvec2 { flex.valueOf<attr::FlexLayout::colGap>(), flex.valueOf<attr::FlexLayout::rowGap>() };

	const auto space = getLayoutState().computeSize - math::fvec2 { padding.x + padding.z, padding.y + padding.z };

	const auto& slot = std::get<2>(getLayoutAttributes().attributeSets);

	auto min = math::fvec2 {};
	auto max = math::fvec2 {};

	if (slot.valueOf<attr::BoxLayout::minWidth>().type == ReflowUnitType::eAbsolute) {
		min.x = slot.valueOf<attr::BoxLayout::minWidth>().value;

	} else if (slot.valueOf<attr::BoxLayout::minWidth>().type == ReflowUnitType::eRelative) {
		const auto maxPerRowCount = std::clamp(
			1.F / slot.valueOf<attr::BoxLayout::minWidth>().value,
			1.F,
			static_cast<f32>(_children.size())
		);
		const auto crunchSpace = space.x - (maxPerRowCount - 1.F) * gapping.x;
		min.x = crunchSpace * slot.valueOf<attr::BoxLayout::minWidth>().value;
	}

	if (slot.valueOf<attr::BoxLayout::maxWidth>().type == ReflowUnitType::eAbsolute) {
		max.x = slot.valueOf<attr::BoxLayout::maxWidth>().value;

	} else if (slot.valueOf<attr::BoxLayout::maxWidth>().type == ReflowUnitType::eRelative) {
		const auto minPerRowCount = std::clamp(
			1.F / slot.valueOf<attr::BoxLayout::maxWidth>().value,
			1.F,
			static_cast<f32>(_children.size())
		);
		const auto crunchSpace = space.x - (minPerRowCount - 1.F) * gapping.x;
		max.x = crunchSpace * slot.valueOf<attr::BoxLayout::maxWidth>().value;
	}

	if (slot.valueOf<attr::BoxLayout::minHeight>().type == ReflowUnitType::eAbsolute) {
		min.y = slot.valueOf<attr::BoxLayout::minHeight>().value;
	}

	if (slot.valueOf<attr::BoxLayout::maxHeight>().type == ReflowUnitType::eAbsolute) {
		max.y = slot.valueOf<attr::BoxLayout::maxHeight>().value;
	}

	return PrefetchSizing {
		min, max
	};

}

PassPrefetchSizing UniformGridLayout::passPrefetchSlotSizing(ReflowAxis axis_, PrefetchSizing prefetch_, math::fvec2 reference_) const {

	const auto& slot = std::get<2>(getLayoutAttributes().attributeSets);
	return {
		algorithm::nextUnit(
			slot.valueOf<attr::BoxLayout::minWidth>(),
			slot.valueOf<attr::BoxLayout::minHeight>(),
			reference_,
			prefetch_.minSizing
		),
		math::compMax(
			algorithm::nextUnit(
				slot.valueOf<attr::BoxLayout::minWidth>(),
				slot.valueOf<attr::BoxLayout::minHeight>(),
				reference_,
				prefetch_.sizing
			),
			prefetch_.sizing
		),
		algorithm::nextUnit(
			slot.valueOf<attr::BoxLayout::maxWidth>(),
			slot.valueOf<attr::BoxLayout::maxHeight>(),
			reference_,
			algorithm::unitAbsMax(
				slot.valueOf<attr::BoxLayout::maxWidth>(),
				slot.valueOf<attr::BoxLayout::maxHeight>()
			)
		)
	};
}

PrefetchSizing UniformGridLayout::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {

	const auto& layout = getLayoutAttributes();
	const auto& box = std::get<0>(layout.attributeSets);
	const auto& flex = std::get<1>(layout.attributeSets);

	const auto pb = algorithm::PrefetchBox {
		.mainAxis = ReflowAxis::eXAxis,
		.min = algorithm::unitAbsMin(box.valueOf<attr::BoxLayout::minWidth>(), box.valueOf<attr::BoxLayout::minHeight>()),
		.max = algorithm::unitAbsMax(box.valueOf<attr::BoxLayout::maxWidth>(), box.valueOf<attr::BoxLayout::maxHeight>()),
		.gapping = { flex.valueOf<attr::FlexLayout::colGap>(), flex.valueOf<attr::FlexLayout::rowGap>() },
		.padding = box.valueOf<attr::BoxLayout::padding>()
	};

	if (pb.min == pb.max) {
		return { pb.min, pb.max };
	}

	const auto slotSizing = axis_ == ReflowAxis::eXAxis ? prefetchSlotSizingX() : prefetchSlotSizingY();
	const auto slotCount = _children.size();

	const auto space = getLayoutState().computeSize - math::fvec2 { pb.padding.x + pb.padding.z, pb.padding.y + pb.padding.z };

	/**/

	const auto minSlotsPerMainAxis = axis_ == ReflowAxis::eYAxis ?
		std::floorf((space.x + pb.gapping.x) / (slotSizing.minSizing.x + pb.gapping.x)) :
		1.F;

	const auto minGapsPerMainAxis = std::max(minSlotsPerMainAxis - 1.F, 0.F);
	const auto minSlotsPerCrossAxis = axis_ == ReflowAxis::eYAxis ?
		std::ceilf(static_cast<f32>(slotCount) / minSlotsPerMainAxis) :
		1.F;
	const auto minGapsPerCrossAxis = std::max(minSlotsPerCrossAxis - 1.F, 0.F);

	const auto minSizing = math::fvec2 {
		slotSizing.minSizing.x * minSlotsPerMainAxis + pb.padding.x + pb.gapping.x * minGapsPerMainAxis + pb.padding.z,
		slotSizing.minSizing.y * minSlotsPerCrossAxis + pb.padding.y + pb.gapping.y * minGapsPerCrossAxis + pb.padding.z
	};

	/**/

	const auto slotsPerMainAxis = axis_ == ReflowAxis::eYAxis ?
		std::floorf((space.x + pb.gapping.x) / (slotSizing.sizing.x + pb.gapping.x)) :
		static_cast<f32>(slotCount);
	const auto gapsPerMainAxis = std::max(slotsPerMainAxis - 1.F, 0.F);
	const auto slotsPerCrossAxis = axis_ == ReflowAxis::eYAxis ?
		std::ceilf(static_cast<f32>(slotCount) / slotsPerMainAxis) :
		1.F;
	const auto gapsPerCrossAxis = std::max(slotsPerCrossAxis - 1.F, 0.F);

	const auto sizing = math::fvec2 {
		slotSizing.sizing.x * slotsPerMainAxis + pb.padding.x + pb.gapping.x * gapsPerMainAxis + pb.padding.z,
		slotSizing.sizing.y * slotsPerCrossAxis + pb.padding.y + pb.gapping.y * gapsPerCrossAxis + pb.padding.w
	};

	return {
		math::compClamp(pb.min, minSizing, pb.max),
		math::compClamp(pb.min, math::compMax(sizing, minSizing), pb.max)
	};
}

PassPrefetchSizing UniformGridLayout::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {

	// TODO: Compute relative sizing of slots and update sizing constraints
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

math::fvec2 UniformGridLayout::computeReferenceSize(ReflowAxis axis_) const {

	const auto& box = std::get<0>(getLayoutAttributes().attributeSets);
	const auto& flex = std::get<1>(getLayoutAttributes().attributeSets);
	const auto& slot = std::get<2>(getLayoutAttributes().attributeSets);
	const auto& padding = box.valueOf<attr::BoxLayout::padding>();

	auto min = algorithm::unitAbsMin(slot.valueOf<attr::BoxLayout::minWidth>(), slot.valueOf<attr::BoxLayout::minHeight>());
	auto max = algorithm::unitAbsMax(slot.valueOf<attr::BoxLayout::maxWidth>(), slot.valueOf<attr::BoxLayout::maxHeight>());

	const auto space = _layoutState.computeSize - math::fvec2 {
		(padding.x + padding.z),
		(padding.y + padding.w)
	};

	if (axis_ == ReflowAxis::eXAxis) {
		if (slot.valueOf<attr::BoxLayout::minWidth>().type == ReflowUnitType::eRelative) {
			const auto intendedPerRowCount = std::clamp(
				1.F / slot.valueOf<attr::BoxLayout::minWidth>().value,
				1.F,
				static_cast<f32>(_children.size())
			);
			const auto crunchSpace = space.x - (intendedPerRowCount - 1.F) * flex.valueOf<attr::FlexLayout::colGap>();
			min.x = crunchSpace * slot.valueOf<attr::BoxLayout::minWidth>().value;
		}

		if (slot.valueOf<attr::BoxLayout::maxWidth>().type == ReflowUnitType::eRelative) {
			const auto intendedPerRowCount = std::clamp(
				1.F / slot.valueOf<attr::BoxLayout::maxWidth>().value,
				1.F,
				static_cast<f32>(_children.size())
			);
			const auto crunchSpace = space.x - (intendedPerRowCount - 1.F) * flex.valueOf<attr::FlexLayout::colGap>();
			max.x = crunchSpace * slot.valueOf<attr::BoxLayout::maxWidth>().value;
		}
	}

	/**
	 * Attention: Using relative height can possibly blow the sizing constraints
	 */
	if (axis_ == ReflowAxis::eYAxis) {
		const auto rowGap = flex.valueOf<attr::FlexLayout::rowGap>();

		if (slot.valueOf<attr::BoxLayout::minHeight>().type == ReflowUnitType::eRelative) {
			const auto intendedRowCount = std::clamp(
				1.F / slot.valueOf<attr::BoxLayout::minHeight>().value,
				1.F,
				static_cast<f32>(_children.size())
			);
			const auto crunchSpace = space.y - (intendedRowCount - 1.F) * rowGap;
			min.y = crunchSpace * slot.valueOf<attr::BoxLayout::minHeight>().value;
		}

		if (slot.valueOf<attr::BoxLayout::maxHeight>().type == ReflowUnitType::eRelative) {
			const auto intendedRowCount = std::clamp(
				1.F / slot.valueOf<attr::BoxLayout::maxHeight>().value,
				1.F,
				static_cast<f32>(_children.size())
			);
			const auto crunchSpace = space.y - (intendedRowCount - 1.F) * rowGap;
			min.y = crunchSpace * slot.valueOf<attr::BoxLayout::maxHeight>().value;
		}
	}

	return math::compMin(math::compMax(min, max), space);
}

void UniformGridLayout::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {

	const auto& box = std::get<0>(getLayoutAttributes().attributeSets);
	const auto& flex = std::get<1>(getLayoutAttributes().attributeSets);

	const auto pb = algorithm::PrefetchBox {
		.mainAxis = ReflowAxis::eXAxis,
		.min = algorithm::unitAbsMin(box.valueOf<attr::BoxLayout::minWidth>(), box.valueOf<attr::BoxLayout::minHeight>()),
		.max = algorithm::unitAbsMax(box.valueOf<attr::BoxLayout::maxWidth>(), box.valueOf<attr::BoxLayout::maxHeight>()),
		.gapping = { flex.valueOf<attr::FlexLayout::colGap>(), flex.valueOf<attr::FlexLayout::rowGap>() },
		.padding = box.valueOf<attr::BoxLayout::padding>()
	};

	const auto space = getLayoutState().computeSize - math::fvec2 { pb.padding.x + pb.padding.z, pb.padding.y + pb.padding.z };
	const auto slotSizing = prefetchSlotSizingY();

	/**/

	const auto maxSlotsPerMainAxis = (space.x + pb.gapping.x) / (slotSizing.minSizing.x + pb.gapping.x);
	const auto maxSlotsPerCrossAxis = (space.y + pb.gapping.y) / (slotSizing.minSizing.y + pb.gapping.y);
	const auto wantedSlotsPerMainAxis = (space.x + pb.gapping.x) / (slotSizing.sizing.x + pb.gapping.x);
	const auto wantedSlotsPerCrossAxis = (space.y + pb.gapping.y) / (slotSizing.sizing.y + pb.gapping.y);

	const auto slotsPerMainAxis = std::max(maxSlotsPerMainAxis, wantedSlotsPerMainAxis);
	const auto slotsPerCrossAxis = (slotsPerMainAxis == maxSlotsPerCrossAxis) ? maxSlotsPerCrossAxis : wantedSlotsPerCrossAxis;

	const auto perSlotSizing = ((space + pb.gapping) / math::fvec2 { slotsPerMainAxis, slotsPerCrossAxis }) - pb.gapping;

	/**/

	for (const auto& child : *children()) {

		if (axis_ == ReflowAxis::eXAxis) {
			auto& layout = child->getLayoutState();
			layout.computeSize.x = perSlotSizing.x;

		} else {
			auto& layout = child->getLayoutState();
			layout.computeSize.y = perSlotSizing.y;
		}

	}
}

void UniformGridLayout::applyLayout(ref<ReflowState> state_) {

	const auto& box = std::get<0>(getLayoutAttributes().attributeSets);
	const auto& flex = std::get<1>(getLayoutAttributes().attributeSets);

	const auto pb = algorithm::PrefetchBox {
		.mainAxis = ReflowAxis::eXAxis,
		.min = algorithm::unitAbsMin(box.valueOf<attr::BoxLayout::minWidth>(), box.valueOf<attr::BoxLayout::minHeight>()),
		.max = algorithm::unitAbsMax(box.valueOf<attr::BoxLayout::maxWidth>(), box.valueOf<attr::BoxLayout::maxHeight>()),
		.gapping = { flex.valueOf<attr::FlexLayout::colGap>(), flex.valueOf<attr::FlexLayout::rowGap>() },
		.padding = box.valueOf<attr::BoxLayout::padding>()
	};

	const auto slotSizing = prefetchSlotSizingY();

	const auto shift = getLayoutState().layoutOffset + math::fvec2 { pb.padding.x, pb.padding.y };
	const auto space = getLayoutState().layoutSize - math::fvec2 { pb.padding.x + pb.padding.z, pb.padding.y + pb.padding.z };

	/**/

	const auto maxSlotsPerMainAxis = (space.x + pb.gapping.x) / (slotSizing.minSizing.x + pb.gapping.x);
	const auto maxSlotsPerCrossAxis = (space.y + pb.gapping.y) / (slotSizing.minSizing.y + pb.gapping.y);
	const auto wantedSlotsPerMainAxis = (space.x + pb.gapping.x) / (slotSizing.sizing.x + pb.gapping.x);
	const auto wantedSlotsPerCrossAxis = (space.y + pb.gapping.y) / (slotSizing.sizing.y + pb.gapping.y);

	const auto slotsPerMainAxis = std::max(maxSlotsPerMainAxis, wantedSlotsPerMainAxis);
	const auto slotsPerCrossAxis = (slotsPerMainAxis == maxSlotsPerCrossAxis) ? maxSlotsPerCrossAxis : wantedSlotsPerCrossAxis;

	const auto perSlotSizing = ((space + pb.gapping) / math::fvec2 { slotsPerMainAxis, slotsPerCrossAxis }) - pb.gapping;

	/**/

	const auto lineLimit = space.x;
	auto line = u16 { 0uL };
	auto lineOccupied = 0.F;
	auto offset = shift;

	for (const auto& child : *children()) {

		if (lineOccupied + perSlotSizing.x > lineLimit) {
			++line;
			lineOccupied = 0.F;
			offset = shift + math::fvec2 { 0.F, static_cast<f32>(line) * (perSlotSizing.y + pb.gapping.y) };
		}

		auto& layout = child->getLayoutState();
		layout.layoutOffset = offset + math::fvec2 { lineOccupied, 0.F };
		layout.layoutSize = perSlotSizing;

		lineOccupied += (perSlotSizing.x + pb.gapping.x);
	}
}

math::fvec2 UniformGridLayout::getGrowFactor() const noexcept {
	return {
		std::get<0>(getLayoutAttributes().attributeSets).valueOf<attr::BoxLayout::widthGrow>(),
		std::get<0>(getLayoutAttributes().attributeSets).valueOf<attr::BoxLayout::heightGrow>()
	};
}

math::fvec2 UniformGridLayout::getShrinkFactor() const noexcept {
	return {
		std::get<0>(getLayoutAttributes().attributeSets).valueOf<attr::BoxLayout::widthShrink>(),
		std::get<0>(getLayoutAttributes().attributeSets).valueOf<attr::BoxLayout::heightShrink>()
	};
}
