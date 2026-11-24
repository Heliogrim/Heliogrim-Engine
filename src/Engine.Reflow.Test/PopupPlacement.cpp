#include "pch.h"

/**/

import Engine.Common;
import Engine.Reflow;

using namespace hg::engine::reflow;
using namespace hg;

namespace ReflowModule {
	constexpr static math::vec2 client { 320.F, 240.F };

	TEST(PopupPlacement, FitsBelow) {
		const PopupAnchor anchor { { 10.F, 10.F }, { 50.F, 20.F }, PopupSide::eBelow, PopupAlign::eStart };
		const auto pos = placePopup(anchor, { 100.F, 50.F }, client);
		EXPECT_EQ(pos.x, 10.F);
		EXPECT_EQ(pos.y, 30.F);
	}

	TEST(PopupPlacement, FlipsAbove) {
		const PopupAnchor anchor { { 10.F, 220.F }, { 50.F, 20.F }, PopupSide::eBelow, PopupAlign::eStart };
		const auto pos = placePopup(anchor, { 100.F, 50.F }, client);
		EXPECT_EQ(pos.x, 10.F);
		EXPECT_EQ(pos.y, 170.F);
	}

	TEST(PopupPlacement, ShiftsAlongEdge) {
		const PopupAnchor anchor { { 300.F, 10.F }, { 20.F, 20.F }, PopupSide::eBelow, PopupAlign::eStart };
		const auto pos = placePopup(anchor, { 100.F, 50.F }, client);
		EXPECT_EQ(pos.x, 220.F);
		EXPECT_EQ(pos.y, 30.F);
	}

	TEST(PopupPlacement, PointerAnchor) {
		const PopupAnchor anchor { { 5.F, 5.F }, { 0.F, 0.F }, PopupSide::eRight, PopupAlign::eStart };
		const auto pos = placePopup(anchor, { 100.F, 50.F }, client);
		EXPECT_EQ(pos.x, 5.F);
		EXPECT_EQ(pos.y, 5.F);
	}

	TEST(PopupPlacement, LargerThanWindowPinned) {
		const PopupAnchor anchor { { 10.F, 10.F }, { 50.F, 20.F }, PopupSide::eBelow, PopupAlign::eStart };
		const auto pos = placePopup(anchor, { 400.F, 300.F }, client);
		EXPECT_EQ(pos.x, 0.F);
		EXPECT_EQ(pos.y, 0.F);
	}

	TEST(PopupPlacement, AlignCenter) {
		const PopupAnchor anchor { { 100.F, 10.F }, { 60.F, 20.F }, PopupSide::eBelow, PopupAlign::eCenter };
		const auto pos = placePopup(anchor, { 100.F, 50.F }, client);
		EXPECT_EQ(pos.x, 80.F);
		EXPECT_EQ(pos.y, 30.F);
	}

	TEST(PopupPlacement, AlignEnd) {
		const PopupAnchor anchor { { 100.F, 10.F }, { 60.F, 20.F }, PopupSide::eBelow, PopupAlign::eEnd };
		const auto pos = placePopup(anchor, { 100.F, 50.F }, client);
		EXPECT_EQ(pos.x, 60.F);
		EXPECT_EQ(pos.y, 30.F);
	}
}
