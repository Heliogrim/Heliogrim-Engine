#include "UniformGridPanel.hpp"

#include <Engine.Logging/Logger.hpp>

#include "../Algorithm/Flex.hpp"
#include "../Algorithm/FlexState.hpp"
#include "../Layout/Style.hpp"

using namespace hg::engine::reflow;
using namespace hg;

UniformGridPanel::UniformGridPanel() :
	GridPanel(),
	attr(
		Attributes {
			.minWidth = { this, { ReflowUnitType::eAuto, 0.F } },
			.width = { this, { ReflowUnitType::eAuto, 0.F } },
			.maxWidth = { this, { ReflowUnitType::eAuto, 0.F } },
			.minHeight = { this, { ReflowUnitType::eAuto, 0.F } },
			.height = { this, { ReflowUnitType::eAuto, 0.F } },
			.maxHeight = { this, { ReflowUnitType::eAuto, 0.F } },
			.padding = { this, Padding { 0.F } },
			.orientation = { this, algorithm::FlexLineOrientation::eHorizontal },
			.justify = { this, ReflowSpacing::eStart },
			.align = { this, ReflowAlignment::eStart },
			.colGap = { this, 0.F },
			.rowGap = { this, 0.F },
			.slot = Attributes::SlotAttributes {
				.width = { this, { ReflowUnitType::eAuto, 0.F } },
				.height = { this, { ReflowUnitType::eAuto, 0.F } },
				.padding = { this, Padding { 0.F } },
				.justify = { this, ReflowAlignment::eCenter },
				.align = { this, ReflowAlignment::eCenter }
			}
		}
	) {}

UniformGridPanel::~UniformGridPanel() = default;

string UniformGridPanel::getTag() const noexcept {
	return std::format(R"(UniformGridPanel <{:#x}>)", reinterpret_cast<u64>(this));
}

math::vec2 UniformGridPanel::computeGridItemSize() const {

	math::vec2 itemSize { 0.F };
	const bool hasAutoSize = attr.slot.width->type == ReflowUnitType::eAuto ||
		attr.slot.height->type == ReflowUnitType::eAuto;

	/**/

	if (hasAutoSize) {
		for (const auto& child : *children()) {
			itemSize = math::compMax<float>(itemSize, child->getDesiredSize());
		}
	}

	/**/

	if (attr.slot.width->type == ReflowUnitType::eAbsolute) {
		itemSize.x = attr.slot.width->value;
	}

	if (attr.slot.height->type == ReflowUnitType::eAbsolute) {
		itemSize.y = attr.slot.height->value;
	}

	/**/

	return itemSize;
}

math::vec2 UniformGridPanel::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

	const auto gridItemSize = computeGridItemSize();
	const auto gridItemCount = children()->size();

	const auto itemSize = gridItemSize * static_cast<float>(gridItemCount);
	const auto gapping = math::vec2 { attr.colGap.getValue(), attr.rowGap.getValue() }
		* static_cast<float>(MAX(gridItemCount - 1, 0));

	const auto innerSize = attr.orientation.getValue() == algorithm::FlexLineOrientation::eVertical ?
		                       math::vec2 { gridItemSize.x, itemSize.y + gapping.y } :
		                       math::vec2 { itemSize.x + gapping.x, gridItemSize.y };

	return layout::innerToOuterSize(attr, innerSize);
}

math::vec2 UniformGridPanel::computeDesiredSize(cref<ReflowPassState> passState_) const {

	math::vec2 desired = getDesiredSize();
	if (attr.width->type == ReflowUnitType::eRelative) {
		desired.x = passState_.referenceSize.x * attr.width->value;
	}
	if (attr.height->type == ReflowUnitType::eRelative) {
		desired.y = passState_.referenceSize.y * attr.height->value;
	}

	/**/

	const auto gridItemSize = computeGridItemSize();
	const auto gridItemCount = children()->size();

	const auto compoundSize = gridItemSize + math::vec2 { attr.colGap.getValue(), attr.rowGap.getValue() };
	const auto itemsPerAxis = layout::outerToInnerSize(attr, desired) / compoundSize;

	math::vec2 innerSize { 0.F };
	if (attr.orientation.getValue() == algorithm::FlexLineOrientation::eVertical) {

		const u32 itemPerLine = MAX(static_cast<u32>(math::floorf(itemsPerAxis.y)), 1uL);
		const u32 lineCount = gridItemCount / itemPerLine + (gridItemCount % itemPerLine == 0 ? 0uL : 1uL);

		innerSize.x = compoundSize.x * static_cast<float>(lineCount) - attr.colGap.getValue();
		innerSize.y = compoundSize.y * static_cast<float>(itemPerLine) - attr.rowGap.getValue();

	} else {

		const u32 itemPerLine = MAX(static_cast<u32>(math::floorf(itemsPerAxis.x)), 1uL);
		const u32 lineCount = gridItemCount / itemPerLine + (gridItemCount % itemPerLine == 0 ? 0uL : 1uL);

		innerSize.x = compoundSize.x * static_cast<float>(itemPerLine) - attr.colGap.getValue();
		innerSize.y = compoundSize.y * static_cast<float>(lineCount) - attr.rowGap.getValue();

	}

	const auto gridContentSize = layout::innerToOuterSize(attr, innerSize);

	/**/

	return layout::clampSize(
		attr,
		passState_.layoutSize,
		math::vec2 {
			attr.width->type == ReflowUnitType::eAuto ? gridContentSize.x : desired.x,
			attr.height->type == ReflowUnitType::eAuto ? gridContentSize.y : desired.y
		}
	);
}

void UniformGridPanel::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

	const auto innerSize = layout::outerToInnerSize(attr, ctx_.localSize);
	const bool isVertical = attr.orientation.getValue() == algorithm::FlexLineOrientation::eVertical;

	/**/

	const auto gridItemSize = computeGridItemSize();
	const auto gridItemCount = children()->size();

	const auto compoundSize = gridItemSize + math::vec2 { attr.colGap.getValue(), attr.rowGap.getValue() };
	const auto itemsPerAxis = innerSize / compoundSize;
	math::uivec2 lineLayout { 0 };

	if (attr.orientation.getValue() == algorithm::FlexLineOrientation::eVertical) {
		lineLayout.x = MAX(static_cast<u32>(math::floorf(itemsPerAxis.y)), 1uL);
		lineLayout.y = gridItemCount / lineLayout.x + (gridItemCount % lineLayout.x == 0 ? 0uL : 1uL);
	} else {
		lineLayout.x = MAX(static_cast<u32>(math::floorf(itemsPerAxis.x)), 1uL);
		lineLayout.y = gridItemCount / lineLayout.x + (gridItemCount % lineLayout.x == 0 ? 0uL : 1uL);
	}

	const math::vec2 mainLineOff = isVertical ? math::vec2 { 0.F, compoundSize.y } : math::vec2 { compoundSize.x, 0.F };
	const math::vec2 crossLineOff = compoundSize - mainLineOff;

	const auto& items = *this->children();
	for (size_t i = 0; i < gridItemCount; ++i) {

		const auto line = i / lineLayout.x;
		const auto item = i % lineLayout.x;

		const math::vec2 offset = ctx_.localOffset +
			mainLineOff * static_cast<float>(item) +
			crossLineOff * static_cast<float>(line);

		/**/

		const auto itemState = state_.getStateOf(items[i]);

		itemState->layoutOffset = offset;
		itemState->layoutSize = gridItemSize;
	}
}
