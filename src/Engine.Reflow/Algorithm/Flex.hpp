#pragma once

#include <numeric>

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Math/Coordinates.hpp>

#include "../Children.hpp"
#include "../ReflowState.hpp"
#include "../Widget/Widget.hpp"

#include "../Gapping.hpp"
#include "../Padding.hpp"

namespace hg::engine::reflow {
	class ReflowState;
	class Widget;
	class Children;
}

namespace hg::engine::reflow::algorithm {
	struct PrefetchBox {
		const ReflowAxis mainAxis;
		const math::fvec2 min;
		const math::fvec2 max;
		/**/
		const Gapping gapping;
		const Padding padding;
	};

	struct PrefetchResult : PrefetchSizing {
		// Inherit math::fvec2 min;
		// Inherit math::fvec2 size;
	};

	struct ComputeBox {
		const ReflowAxis mainAxis;
		const math::fvec2 size;
		/**/
		const Gapping gapping;
		const Padding padding;
	};

	struct LayoutBox {
		const ReflowAxis mainAxis;
		const math::fvec2 anchor;
		const math::fvec2 span;
		/**/
		const Gapping gapping;
		const Padding padding;
		/**/
		ReflowAlignment align;
		ReflowSpacing justify;
	};

	/**/

	[[nodiscard]] constexpr inline math::fvec2 unitAbsMax(ReflowUnit width_, ReflowUnit height_) noexcept {
		return math::fvec2 {
			width_.type == ReflowUnitType::eAbsolute ? width_.value : std::numeric_limits<f32>::infinity(),
			height_.type == ReflowUnitType::eAbsolute ? height_.value : std::numeric_limits<f32>::infinity()
		};
	}

	[[nodiscard]] constexpr inline math::fvec2 unitAbsMin(ReflowUnit width_, ReflowUnit height_) noexcept {
		return math::fvec2 {
			width_.type == ReflowUnitType::eAbsolute ? width_.value : 0.F,
			height_.type == ReflowUnitType::eAbsolute ? height_.value : 0.F
		};
	}

	[[nodiscard]] constexpr inline PrefetchResult prefetch(
		ReflowAxis axis_,
		ref<const PrefetchBox> box_,
		ptr<const Children> children_
	) noexcept {

		if (box_.min == box_.max) {
			return { box_.min, box_.max };
		}

		auto minSize = math::fvec2 {};
		auto min = math::fvec2 {};
		auto size = math::fvec2 {};
		auto max = math::fvec2 {};
		for (const auto& child : *children_) {
			const auto& state = child->getLayoutState();

			minSize += state.prefetchMinSize;
			min = math::compMax(min, state.prefetchMinSize);

			size += state.prefetchSize;
			max = math::compMax(max, state.prefetchSize);
		}

		const auto gaps = children_->empty() ? 0uLL : children_->size() - 1uLL;
		const auto minSizing = (box_.mainAxis == ReflowAxis::eXAxis) ?
			math::fvec2 {
				minSize.x + box_.padding.x + box_.padding.z + (box_.gapping.x * gaps),
				min.y + box_.padding.y + box_.padding.w
			} :
			math::fvec2 {
				min.x + box_.padding.x + box_.padding.z,
				minSize.y + box_.padding.y + box_.padding.w + (box_.gapping.y * gaps)
			};
		const auto sizing = (box_.mainAxis == ReflowAxis::eXAxis) ?
			math::fvec2 {
				size.x + box_.padding.x + box_.padding.z + (box_.gapping.x * gaps),
				max.y + box_.padding.y + box_.padding.w
			} :
			math::fvec2 {
				max.x + box_.padding.x + box_.padding.z,
				size.y + box_.padding.y + box_.padding.w + (box_.gapping.y * gaps),
			};

		return {
			math::compClamp(box_.min, minSizing, box_.max),
			math::compClamp(box_.min, sizing, box_.max)
		};
	}

	/**/

	[[nodiscard]] constexpr inline math::fvec2 nextUnit(
		ReflowUnit width_,
		ReflowUnit height_,
		math::fvec2 reference_,
		math::fvec2 previous_
	) noexcept {
		return math::fvec2 {
			width_.type != ReflowUnitType::eRelative ? previous_.x : width_.value * reference_.x,
			height_.type != ReflowUnitType::eRelative ? previous_.y : height_.value * reference_.y
		};
	}

	/**/

	constexpr inline void compute(
		_In_ ReflowAxis axis_,
		_In_ ref<const ComputeBox> box_,
		_Inout_ ptr<const Children> children_
	) noexcept {

		const auto isMainAxis = axis_ == box_.mainAxis;
		const auto isCrossAxis = axis_ != box_.mainAxis;

		const auto gaps = children_->empty() ? 0uLL : children_->size() - 1uLL;
		const auto space = box_.size -
			math::vec2 { box_.padding.x + box_.padding.z, box_.padding.y + box_.padding.z } -
			(box_.mainAxis == ReflowAxis::eXAxis ? math::fvec2 { box_.gapping.x, 0.F } : math::fvec2 { 0.F, box_.gapping.y }) * gaps;

		auto size = math::fvec2 {};
		auto max = math::fvec2 {};

		for (const auto& child : *children_) {
			const auto& state = child->getLayoutState();
			size += state.prefetchSize;
			max = math::compMax(max, state.prefetchSize);
		}

		const auto sizing = (box_.mainAxis == ReflowAxis::eXAxis) ?
			math::fvec2 {
				size.x + box_.padding.x + box_.padding.z + (box_.gapping.x * gaps),
				max.y + box_.padding.y + box_.padding.w
			} :
			math::fvec2 {
				max.x + box_.padding.x + box_.padding.z,
				size.y + box_.padding.y + box_.padding.w + (box_.gapping.y * gaps),
			};

		/**/

		const auto diff = box_.size - sizing;
		auto underflow = 0.F;
		auto overflow = 0.F;

		if (axis_ == ReflowAxis::eXAxis && isMainAxis && diff.x < 0.F) {
			overflow = -diff.x;
		}

		if (axis_ == ReflowAxis::eXAxis && isMainAxis && diff.x > 0.F) {
			underflow = diff.x;
		}

		if (axis_ == ReflowAxis::eYAxis && isMainAxis && diff.y < 0.F) {
			overflow = -diff.y;
		}

		if (axis_ == ReflowAxis::eYAxis && isMainAxis && diff.y > 0.F) {
			underflow = diff.y;
		}

		/**/

		struct FlexCaps {
			u16 index;
			f32 range;
			f32 factor;
		};
		auto capabilities = Vector<FlexCaps> {};
		auto factorSum = f32 { 0.F };

		u16 index = 0;
		for (auto& child : *children_) {

			auto& layout = child->getLayoutState();
			auto localShrinkRange = layout.prefetchSize - layout.prefetchMinSize;
			auto localGrowRange = math::compMin(space, layout.prefetchMaxSize) - layout.prefetchSize;

			const auto localShrinkFactor = child->getShrinkFactor();
			const auto localGrowFactor = child->getGrowFactor();

			/* Fetch dynamic resizing capabilities */

			if (axis_ == ReflowAxis::eXAxis) {

				if (localShrinkFactor.x > 0.F && overflow > 0.F && localShrinkRange.x > 0.F) {
					capabilities.emplace_back(index, std::max(localShrinkRange.x, overflow), localShrinkFactor.x);
					factorSum += localShrinkFactor.x;
				}

				if (localGrowFactor.x > 0.F && underflow > 0.F && localGrowRange.x > 0.F) {
					capabilities.emplace_back(index, std::max(localGrowRange.x, underflow), localGrowFactor.x);
					factorSum += localGrowFactor.x;
				}

			}

			if (axis_ == ReflowAxis::eYAxis) {

				if (localShrinkFactor.y > 0.F && overflow > 0.F && localShrinkRange.y > 0.F) {
					capabilities.emplace_back(index, std::max(localShrinkRange.y, overflow), localShrinkFactor.y);
					factorSum += localShrinkFactor.y;
				}

				if (localGrowFactor.y > 0.F && underflow > 0.F && localGrowRange.y > 0.F) {
					capabilities.emplace_back(index, std::min(localGrowRange.y, underflow), localGrowFactor.y);
					factorSum += localGrowFactor.y;
				}

			}

			/* Eagerly apply cross-axis manipulation, as we can drop an additional cycle for it. */

			const auto perChildDiff = space - layout.prefetchSize;
			auto crossAxisChange = 0.F;

			if (isCrossAxis && (
				(axis_ == ReflowAxis::eXAxis && localGrowRange.x > 0.F && localGrowFactor.x > 0.F) ||
				(axis_ == ReflowAxis::eYAxis && localGrowRange.y > 0.F && localGrowFactor.y > 0.F)
			)) {

				if (axis_ == ReflowAxis::eXAxis && perChildDiff.x > 0.F) {
					crossAxisChange += std::min(localGrowRange.x, perChildDiff.x);
				}

				if (axis_ == ReflowAxis::eYAxis && perChildDiff.y > 0.F) {
					crossAxisChange += std::min(localGrowRange.y, perChildDiff.y);
				}

			}

			if (isCrossAxis && (
				(axis_ == ReflowAxis::eXAxis && localShrinkRange.x > 0.F && localShrinkFactor.x > 0.F) ||
				(axis_ == ReflowAxis::eYAxis && localShrinkRange.y > 0.F && localShrinkFactor.y > 0.F)
			)) {

				if (axis_ == ReflowAxis::eXAxis && perChildDiff.x < 0.F) {
					crossAxisChange -= std::min(localShrinkRange.x, -perChildDiff.x);
				}

				if (axis_ == ReflowAxis::eYAxis && perChildDiff.y < 0.F) {
					crossAxisChange -= std::min(localShrinkRange.y, -perChildDiff.y);
				}

			}

			/**/

			if (axis_ == ReflowAxis::eXAxis) {
				layout.computeSize.x = layout.prefetchSize.x + crossAxisChange;

				// Attention: May hard limit for cross-axis scaling, forcing max-extend
				//layout.computeSize.x = std::min(layout.computeSize.x, space.x);

			} else {
				layout.computeSize.y = layout.prefetchSize.y + crossAxisChange;

				// Attention: May hard limit for cross-axis scaling, forcing max-extend
				//layout.computeSize.y = std::min(layout.computeSize.y, space.y);
			}

			/**/

			++index;
		}

		/**/

		std::ranges::sort(capabilities, std::ranges::greater {}, [](const auto& cap_) { return cap_.range; });

		while (overflow && not capabilities.empty()) {

			const auto nextStep = std::min(overflow, capabilities.back().range);
			const auto fractionPerRange = nextStep / factorSum;

			overflow -= nextStep;

			if (fractionPerRange <= 0.F) {
				capabilities.pop_back();
				continue;
			}

			for (auto& cap : capabilities) {

				auto& layout = (*children_)[cap.index]->getLayoutState();
				auto consume = cap.factor * fractionPerRange;
				cap.range -= consume;

				if (axis_ == ReflowAxis::eXAxis) {
					layout.computeSize.x -= consume;
				} else {
					layout.computeSize.y -= consume;
				}

			}

			capabilities.pop_back();
		}

		while (underflow && not capabilities.empty()) {

			const auto nextStep = std::min(underflow, capabilities.back().range);
			const auto fractionPerRange = nextStep / factorSum;

			underflow -= nextStep;

			if (fractionPerRange <= 0.F) {
				capabilities.pop_back();
				continue;
			}

			for (auto& cap : capabilities) {

				auto& layout = (*children_)[cap.index]->getLayoutState();
				auto consume = cap.factor * fractionPerRange;
				cap.range -= consume;

				if (axis_ == ReflowAxis::eXAxis) {
					layout.computeSize.x += consume;
				} else {
					layout.computeSize.y += consume;
				}

			}

			capabilities.pop_back();
		}

	}

	/**/

	constexpr inline void layout(
		_In_ ref<const LayoutBox> box_,
		_Inout_ ptr<const Children> children_
	) {

		auto occupiedAlongAxis = 0.F;
		if (box_.mainAxis == ReflowAxis::eXAxis) {
			for (const auto& child : *children_) { occupiedAlongAxis += child->getLayoutState().computeSize.x; }
		} else {
			for (const auto& child : *children_) { occupiedAlongAxis += child->getLayoutState().computeSize.y; }
		}

		const auto gaps = children_->empty() ? 0uLL : children_->size() - 1uLL;
		const auto spaceAlongAxis = (box_.mainAxis == ReflowAxis::eXAxis) ?
			(box_.span.x - box_.padding.x - box_.padding.z - box_.gapping.x * static_cast<f32>(gaps)) :
			(box_.span.y - box_.padding.y - box_.padding.w - box_.gapping.y * static_cast<f32>(gaps));

		const auto unoccupiedAlongAxis = spaceAlongAxis - occupiedAlongAxis;
		auto start = box_.anchor + math::vec2 { box_.padding.x, box_.padding.y };
		auto gapAlongAxis = box_.mainAxis == ReflowAxis::eXAxis ? box_.gapping.x : box_.gapping.y;

		switch (box_.justify) {
			case ReflowSpacing::eStart: {
				break;
			}
			case ReflowSpacing::eEnd: {
				if (box_.mainAxis == ReflowAxis::eXAxis) {
					start.x += unoccupiedAlongAxis;
				} else {
					start.y += unoccupiedAlongAxis;
				}
				break;
			}
			case ReflowSpacing::eSpaceAround: {
				if (box_.mainAxis == ReflowAxis::eXAxis) {
					start.x += unoccupiedAlongAxis / 2.F;
				} else {
					start.y += unoccupiedAlongAxis / 2.F;
				}
				break;
			}
			case ReflowSpacing::eSpaceBetween: {
				gapAlongAxis += (unoccupiedAlongAxis / static_cast<f32>(gaps));
				break;
			}
			case ReflowSpacing::eSpaceEvenly: {
				const auto gaps = children_->empty() ? 0uLL : children_->size() - 1uLL;
				const auto fraction = unoccupiedAlongAxis / static_cast<f32>(gaps + 2uLL);
				gapAlongAxis += fraction;

				if (box_.mainAxis == ReflowAxis::eXAxis) {
					start.x += fraction;
				} else {
					start.y += fraction;
				}
				break;
			}
		}

		/**/

		const auto spaceAcrossAxis = (box_.mainAxis == ReflowAxis::eYAxis) ?
			(box_.span.x - box_.padding.x - box_.padding.z) :
			(box_.span.y - box_.padding.y - box_.padding.w);

		auto offset = start;
		for (const auto& child : *children_) {

			auto& layout = child->getLayoutState();
			layout.layoutOffset = offset;
			layout.layoutSize = layout.computeSize;

			/**/

			if (box_.mainAxis == ReflowAxis::eXAxis) {

				const auto unoccupiedAcrossAxis = spaceAcrossAxis - layout.computeSize.y;
				switch (box_.align) {
					case ReflowAlignment::eStart: {
						break;
					}
					case ReflowAlignment::eCenter: {
						layout.layoutOffset.y += unoccupiedAcrossAxis / 2.F;
						break;
					}
					case ReflowAlignment::eEnd: {
						layout.layoutOffset.y += unoccupiedAcrossAxis;
					}
				}

			} else {

				const auto unoccupiedAcrossAxis = spaceAcrossAxis - layout.computeSize.x;
				switch (box_.align) {
					case ReflowAlignment::eStart: {
						break;
					}
					case ReflowAlignment::eCenter: {
						layout.layoutOffset.x += unoccupiedAcrossAxis / 2.F;
						break;
					}
					case ReflowAlignment::eEnd: {
						layout.layoutOffset.x += unoccupiedAcrossAxis;
					}
				}
			}

			/**/

			if (box_.mainAxis == ReflowAxis::eXAxis) {
				offset.x += gapAlongAxis + layout.computeSize.x;
			} else {
				offset.y += gapAlongAxis + layout.computeSize.y;
			}

		}

	}
}
