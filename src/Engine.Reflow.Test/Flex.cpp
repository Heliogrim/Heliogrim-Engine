#include "pch.h"

#include <cmath>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/BoxLayout.hpp>
#include <Engine.Reflow/Algorithm/Flex.hpp>

using namespace hg;
using namespace hg::engine::reflow;

namespace ReflowModule {
	namespace {
		struct FlexFixture :
			public ::testing::Test {
			Children children {};

			void add(const math::fvec2 computeSize_) {
				auto widget = make_sptr<uikit::BoxLayout>();
				widget->getLayoutState().computeSize = computeSize_;
				children.push_back(std::move(widget));
			}

			void addPrefetched(const math::fvec2 minSize_, const math::fvec2 size_) {
				auto widget = make_sptr<uikit::BoxLayout>();
				auto& state = widget->getLayoutState();
				state.prefetchMinSize = minSize_;
				state.prefetchSize = size_;
				state.computeSize = size_;
				children.push_back(std::move(widget));
			}

			[[nodiscard]] math::fvec2 offsetOf(const size_t idx_) const { return children[idx_]->getLayoutState().layoutOffset; }

			[[nodiscard]] math::fvec2 sizeOf(const size_t idx_) const { return children[idx_]->getLayoutState().layoutSize; }

			[[nodiscard]] static algorithm::LayoutBox box(
				const ReflowAxis axis_,
				const ReflowSpacing justify_,
				const ReflowAlignment align_ = ReflowAlignment::eStart,
				const math::fvec2 gapping_ = { 5.F, 5.F },
				const Padding padding_ = { 0.F, 0.F, 0.F, 0.F }
			) {
				return algorithm::LayoutBox {
					axis_, { 0.F, 0.F }, { 100.F, 50.F }, gapping_, padding_, align_, justify_
				};
			}
		};
	}

	/**/

	TEST(FlexUnits, AbsoluteAndNonAbsoluteBounds) {
		const auto abs = ReflowUnit { ReflowUnitType::eAbsolute, 12.F };
		const auto rel = ReflowUnit { ReflowUnitType::eRelative, 0.5F };

		const auto max = algorithm::unitAbsMax(abs, rel);
		EXPECT_EQ(max.x, 12.F);
		EXPECT_TRUE(std::isinf(max.y));

		const auto min = algorithm::unitAbsMin(abs, rel);
		EXPECT_EQ(min.x, 12.F);
		EXPECT_EQ(min.y, 0.F);
	}

	TEST(FlexUnits, NextUnitScalesRelativeOnly) {
		const auto abs = ReflowUnit { ReflowUnitType::eAbsolute, 12.F };
		const auto rel = ReflowUnit { ReflowUnitType::eRelative, 0.5F };

		const auto next = algorithm::nextUnit(rel, abs, { 200.F, 100.F }, { 7.F, 9.F });
		EXPECT_EQ(next.x, 100.F);
		EXPECT_EQ(next.y, 9.F);
	}

	/**/

	using FlexPrefetch = FlexFixture;

	TEST_F(FlexPrefetch, FixedBoxShortCircuits) {
		addPrefetched({ 1.F, 1.F }, { 50.F, 50.F });

		const algorithm::PrefetchBox box { ReflowAxis::eXAxis, { 30.F, 40.F }, { 30.F, 40.F }, { 3.F, 3.F }, { 1.F, 1.F, 1.F, 1.F } };
		const auto res = algorithm::prefetch(ReflowAxis::eXAxis, box, &children);

		EXPECT_EQ(res.minSizing, (math::fvec2 { 30.F, 40.F }));
		EXPECT_EQ(res.sizing, (math::fvec2 { 30.F, 40.F }));
	}

	TEST_F(FlexPrefetch, MainAxisSumsCrossAxisTakesMaxPlusGapAndPadding) {
		addPrefetched({ 1.F, 1.F }, { 10.F, 5.F });
		addPrefetched({ 2.F, 2.F }, { 20.F, 15.F });

		// padding: x=left, y=top, z=right, w=bottom
		const algorithm::PrefetchBox box { ReflowAxis::eXAxis, { 0.F, 0.F }, { 1000.F, 1000.F }, { 3.F, 3.F }, { 1.F, 2.F, 3.F, 4.F } };
		const auto res = algorithm::prefetch(ReflowAxis::eXAxis, box, &children);

		// min.x = (1 + 2) + (1 + 3) + 3 * 1 gap, min.y = max(1, 2) + (2 + 4)
		EXPECT_EQ(res.minSizing, (math::fvec2 { 10.F, 8.F }));
		// size.x = (10 + 20) + (1 + 3) + 3, size.y = max(5, 15) + (2 + 4)
		EXPECT_EQ(res.sizing, (math::fvec2 { 37.F, 21.F }));
	}

	TEST_F(FlexPrefetch, VerticalAxisSwapsRoles) {
		addPrefetched({ 1.F, 1.F }, { 10.F, 5.F });
		addPrefetched({ 2.F, 2.F }, { 20.F, 15.F });

		const algorithm::PrefetchBox box { ReflowAxis::eYAxis, { 0.F, 0.F }, { 1000.F, 1000.F }, { 3.F, 3.F }, { 0.F, 0.F, 0.F, 0.F } };
		const auto res = algorithm::prefetch(ReflowAxis::eYAxis, box, &children);

		EXPECT_EQ(res.sizing, (math::fvec2 { 20.F, 23.F }));
		EXPECT_EQ(res.minSizing, (math::fvec2 { 2.F, 6.F }));
	}

	TEST_F(FlexPrefetch, ResultIsClampedToBoxBounds) {
		addPrefetched({ 1.F, 1.F }, { 500.F, 500.F });

		const algorithm::PrefetchBox box { ReflowAxis::eXAxis, { 0.F, 0.F }, { 100.F, 80.F }, { 0.F, 0.F }, { 0.F, 0.F, 0.F, 0.F } };
		const auto res = algorithm::prefetch(ReflowAxis::eXAxis, box, &children);

		EXPECT_EQ(res.sizing, (math::fvec2 { 100.F, 80.F }));
	}

	TEST_F(FlexPrefetch, NoChildrenYieldsPaddingOnly) {
		const algorithm::PrefetchBox box { ReflowAxis::eXAxis, { 0.F, 0.F }, { 1000.F, 1000.F }, { 9.F, 9.F }, { 1.F, 2.F, 3.F, 4.F } };
		const auto res = algorithm::prefetch(ReflowAxis::eXAxis, box, &children);

		EXPECT_EQ(res.sizing, (math::fvec2 { 4.F, 6.F }));
	}

	/**/

	using FlexLayout = FlexFixture;

	TEST_F(FlexLayout, StartPlacesWithGap) {
		add({ 10.F, 10.F });
		add({ 20.F, 20.F });
		add({ 30.F, 30.F });

		algorithm::layout(box(ReflowAxis::eXAxis, ReflowSpacing::eStart), &children);

		EXPECT_EQ(offsetOf(0).x, 0.F);
		EXPECT_EQ(offsetOf(1).x, 15.F);
		EXPECT_EQ(offsetOf(2).x, 40.F);
		EXPECT_EQ(sizeOf(1), (math::fvec2 { 20.F, 20.F }));
	}

	TEST_F(FlexLayout, EndPushesToTrailingEdge) {
		add({ 10.F, 10.F });
		add({ 20.F, 20.F });
		add({ 30.F, 30.F });

		algorithm::layout(box(ReflowAxis::eXAxis, ReflowSpacing::eEnd), &children);

		// unoccupied = (100 - 2 * 5) - 60 = 30
		EXPECT_EQ(offsetOf(0).x, 30.F);
		EXPECT_EQ(offsetOf(2).x, 70.F);
	}

	TEST_F(FlexLayout, SpaceAroundCentersGroup) {
		add({ 10.F, 10.F });
		add({ 20.F, 20.F });
		add({ 30.F, 30.F });

		algorithm::layout(box(ReflowAxis::eXAxis, ReflowSpacing::eSpaceAround), &children);

		EXPECT_EQ(offsetOf(0).x, 15.F);
		EXPECT_EQ(offsetOf(2).x, 55.F);
	}

	TEST_F(FlexLayout, SpaceBetweenDistributesGaps) {
		add({ 10.F, 10.F });
		add({ 20.F, 20.F });
		add({ 30.F, 30.F });

		algorithm::layout(box(ReflowAxis::eXAxis, ReflowSpacing::eSpaceBetween), &children);

		EXPECT_EQ(offsetOf(0).x, 0.F);
		EXPECT_EQ(offsetOf(1).x, 30.F);
		EXPECT_EQ(offsetOf(2).x, 70.F);
	}

	TEST_F(FlexLayout, SpaceEvenlyUsesGapsPlusTwoSlots) {
		add({ 10.F, 10.F });
		add({ 20.F, 20.F });
		add({ 30.F, 30.F });

		algorithm::layout(box(ReflowAxis::eXAxis, ReflowSpacing::eSpaceEvenly), &children);

		EXPECT_EQ(offsetOf(0).x, 7.5F);
		EXPECT_EQ(offsetOf(1).x, 30.F);
		EXPECT_EQ(offsetOf(2).x, 62.5F);
	}

	TEST_F(FlexLayout, CrossAxisAlignment) {
		add({ 10.F, 10.F });

		algorithm::layout(box(ReflowAxis::eXAxis, ReflowSpacing::eStart, ReflowAlignment::eCenter), &children);
		EXPECT_EQ(offsetOf(0).y, 20.F);

		algorithm::layout(box(ReflowAxis::eXAxis, ReflowSpacing::eStart, ReflowAlignment::eEnd), &children);
		EXPECT_EQ(offsetOf(0).y, 40.F);

		algorithm::layout(box(ReflowAxis::eXAxis, ReflowSpacing::eStart, ReflowAlignment::eStart), &children);
		EXPECT_EQ(offsetOf(0).y, 0.F);
	}

	TEST_F(FlexLayout, VerticalMainAxisStacksDownwards) {
		add({ 10.F, 10.F });
		add({ 10.F, 20.F });

		algorithm::layout(box(ReflowAxis::eYAxis, ReflowSpacing::eStart, ReflowAlignment::eCenter), &children);

		EXPECT_EQ(offsetOf(0).y, 0.F);
		EXPECT_EQ(offsetOf(1).y, 15.F);
		// across axis: (100 - 0) - 10, centered
		EXPECT_EQ(offsetOf(0).x, 45.F);
	}

	TEST_F(FlexLayout, PaddingShiftsStart) {
		add({ 10.F, 10.F });

		algorithm::layout(
			box(ReflowAxis::eXAxis, ReflowSpacing::eStart, ReflowAlignment::eStart, { 0.F, 0.F }, { 4.F, 6.F, 8.F, 2.F }),
			&children
		);

		EXPECT_EQ(offsetOf(0), (math::fvec2 { 4.F, 6.F }));
	}

	// BUG: SpaceBetween divides by `gaps`, which is 0 for a single child, yielding inf/NaN offsets.
	TEST_F(FlexLayout, DISABLED_SpaceBetweenSingleChildStaysFinite) {
		add({ 10.F, 10.F });

		algorithm::layout(box(ReflowAxis::eXAxis, ReflowSpacing::eSpaceBetween), &children);

		EXPECT_TRUE(std::isfinite(offsetOf(0).x));
	}

	/**/

	using FlexCompute = FlexFixture;

	TEST_F(FlexCompute, MainAxisKeepsPrefetchSizeWithoutFlexFactors) {
		addPrefetched({ 10.F, 10.F }, { 40.F, 20.F });
		addPrefetched({ 10.F, 10.F }, { 40.F, 30.F });

		const algorithm::ComputeBox box { ReflowAxis::eXAxis, { 200.F, 100.F }, { 0.F, 0.F }, { 0.F, 0.F, 0.F, 0.F } };
		algorithm::compute(ReflowAxis::eXAxis, box, std::span<const SharedPtr<Widget>> { children.data(), children.size() });

		EXPECT_EQ(children[0]->getLayoutState().computeSize.x, 40.F);
		EXPECT_EQ(children[1]->getLayoutState().computeSize.x, 40.F);
	}
}
