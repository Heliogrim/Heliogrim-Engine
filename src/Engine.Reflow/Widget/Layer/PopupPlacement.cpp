#include "PopupPlacement.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

using namespace hg::engine::reflow;
using namespace hg;

[[nodiscard]] static bool isVertical(const PopupSide side_) noexcept {
	return side_ == PopupSide::eBelow || side_ == PopupSide::eAbove;
}

[[nodiscard]] static PopupSide opposite(const PopupSide side_) noexcept {
	switch (side_) {
		case PopupSide::eBelow: return PopupSide::eAbove;
		case PopupSide::eAbove: return PopupSide::eBelow;
		case PopupSide::eRight: return PopupSide::eLeft;
		default: return PopupSide::eRight;
	}
}

math::vec2 hg::engine::reflow::placePopup(
	ref<const PopupAnchor> anchor_,
	const math::vec2 popupSize_,
	const math::vec2 clientSize_
) noexcept {

	const auto vertical = isVertical(anchor_.side);
	const auto main = vertical ? 1 : 0;
	const auto cross = vertical ? 0 : 1;

	const auto mainStart = [&](const PopupSide side_) {
		const auto after = side_ == PopupSide::eBelow || side_ == PopupSide::eRight;
		return after ? anchor_.offset[main] + anchor_.size[main] : anchor_.offset[main] - popupSize_[main];
	};
	const auto fits = [&](const float pos_) {
		return pos_ >= 0.F && pos_ + popupSize_[main] <= clientSize_[main];
	};

	auto side = anchor_.side;
	if (not fits(mainStart(side))) {
		const auto flipped = opposite(side);
		if (fits(mainStart(flipped))) {
			side = flipped;

		} else {
			const auto roomAfter = clientSize_[main] - (anchor_.offset[main] + anchor_.size[main]);
			const auto roomBefore = anchor_.offset[main];
			const auto after = roomAfter >= roomBefore;
			side = vertical ?
				(after ? PopupSide::eBelow : PopupSide::eAbove) :
				(after ? PopupSide::eRight : PopupSide::eLeft);
		}
	}

	float crossPos = anchor_.offset[cross];
	if (anchor_.align == PopupAlign::eCenter) {
		crossPos += (anchor_.size[cross] - popupSize_[cross]) * 0.5F;

	} else if (anchor_.align == PopupAlign::eEnd) {
		crossPos += anchor_.size[cross] - popupSize_[cross];
	}

	math::vec2 result {};
	result[main] = mainStart(side);
	result[cross] = crossPos;

	// Shift along the edge into the client area, then pin the top-left (larger than window wins at 0).
	for (auto axis = 0; axis < 2; ++axis) {
		result[axis] = std::max(0.F, std::min(result[axis], clientSize_[axis] - popupSize_[axis]));
	}
	return result;
}
