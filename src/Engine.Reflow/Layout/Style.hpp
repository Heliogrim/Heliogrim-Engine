#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Vector.hpp>

#include <Engine.Reflow.Attributes/Layout/Base/BoxLayoutAttributes.hpp>

#include "../ReflowUnit.hpp"

namespace hg::engine::reflow::layout {
	template <typename Attributes_>
	[[nodiscard]] math::vec2 innerToOuterSize(cref<Attributes_> attributes_, cref<math::vec2> innerSize_) {

		math::vec2 size = innerSize_;

		/**/

		size.x += (attributes_.padding->x + attributes_.padding->z);
		size.y += (attributes_.padding->y + attributes_.padding->w);

		/**/

		return size;
	}

	template <>
	[[nodiscard]] inline math::vec2 innerToOuterSize<BoxLayoutAttributes>(cref<BoxLayoutAttributes> box_, cref<math::vec2> innerSize_) {

		const auto& padding = box_.valueOf<attr::BoxLayout::padding>();
		math::vec2 size = innerSize_;

		/**/

		size.x += (padding.x + padding.z);
		size.y += (padding.y + padding.w);

		/**/

		return size;
	}

	template <typename Attributes_>
	[[nodiscard]] math::vec2 outerToInnerSize(cref<Attributes_> attributes_, cref<math::vec2> outerSize_) {

		math::vec2 size = outerSize_;

		/**/

		size.x -= (attributes_.padding->x + attributes_.padding->z);
		size.y -= (attributes_.padding->y + attributes_.padding->w);

		/**/

		return size;
	}

	template <>
	[[nodiscard]] inline math::vec2 outerToInnerSize<BoxLayoutAttributes>(cref<BoxLayoutAttributes> box_, cref<math::vec2> outerSize_) {

		const auto& padding = box_.valueOf<attr::BoxLayout::padding>();
		math::vec2 size = outerSize_;

		/**/

		size.x -= (padding.x + padding.z);
		size.y -= (padding.y + padding.w);

		/**/

		return size;
	}

	/**/

	template <typename Attributes_>
	[[nodiscard]] math::vec2 innerToOuterOffset(cref<Attributes_> attributes_, cref<math::vec2> innerOffset_) {

		math::vec2 off = innerOffset_;

		/**/

		off.x -= attributes_.padding->x;
		off.y -= attributes_.padding->y;

		/**/

		return off;
	}

	template <>
	[[nodiscard]] inline math::vec2 innerToOuterOffset<BoxLayoutAttributes>(cref<BoxLayoutAttributes> box_, cref<math::vec2> innerOffset_) {

		const auto& padding = box_.valueOf<attr::BoxLayout::padding>();
		math::vec2 off = innerOffset_;

		/**/

		off.x -= padding.x;
		off.y -= padding.y;

		/**/

		return off;
	}

	template <typename Attributes_>
	[[nodiscard]] math::vec2 outerToInnerOffset(cref<Attributes_> attributes_, cref<math::vec2> innerOffset_) {

		math::vec2 off = innerOffset_;

		/**/

		off.x += attributes_.padding->x;
		off.y += attributes_.padding->y;

		/**/

		return off;
	}

	template <>
	[[nodiscard]] inline math::vec2 outerToInnerOffset<BoxLayoutAttributes>(cref<BoxLayoutAttributes> box_, cref<math::vec2> innerOffset_) {

		const auto& padding = box_.valueOf<attr::BoxLayout::padding>();
		math::vec2 off = innerOffset_;

		/**/

		off.x += padding.x;
		off.y += padding.y;

		/**/

		return off;
	}

	/**/

	template <typename Attributes_>
	[[nodiscard]] math::vec2 clampSizeAbs(cref<Attributes_> attributes_, cref<math::vec2> size_) {

		math::vec2 size = size_;

		/**/

		if (attributes_.minWidth->type == ReflowUnitType::eAbsolute) {
			size.x = MAX(size.x, attributes_.minWidth->value);
		}

		if (attributes_.minHeight->type == ReflowUnitType::eAbsolute) {
			size.y = MAX(size.y, attributes_.minHeight->value);
		}

		/**/

		if (attributes_.maxWidth->type == ReflowUnitType::eAbsolute) {
			size.x = MIN(size.x, attributes_.maxWidth->value);
		}

		if (attributes_.maxHeight->type == ReflowUnitType::eAbsolute) {
			size.y = MIN(size.y, attributes_.maxHeight->value);
		}

		/**/

		return size;
	}

	template <>
	[[nodiscard]] inline math::vec2 clampSizeAbs<BoxLayoutAttributes>(cref<BoxLayoutAttributes> box_, cref<math::vec2> size_) {

		math::vec2 size = size_;

		/**/

		if (box_.valueOf<attr::BoxLayout::minWidth>().type == ReflowUnitType::eAbsolute) {
			size.x = MAX(size.x, box_.valueOf<attr::BoxLayout::minWidth>().value);
		}

		if (box_.valueOf<attr::BoxLayout::minHeight>().type == ReflowUnitType::eAbsolute) {
			size.y = MAX(size.y, box_.valueOf<attr::BoxLayout::minHeight>().value);
		}

		/**/

		if (box_.valueOf<attr::BoxLayout::maxWidth>().type == ReflowUnitType::eAbsolute) {
			size.x = MIN(size.x, box_.valueOf<attr::BoxLayout::maxWidth>().value);
		}

		if (box_.valueOf<attr::BoxLayout::maxHeight>().type == ReflowUnitType::eAbsolute) {
			size.y = MIN(size.y, box_.valueOf<attr::BoxLayout::maxHeight>().value);
		}

		/**/

		return size;
	}

	template <typename Attributes_>
	[[nodiscard]] math::vec2 clampSize(
		cref<Attributes_> attributes_,
		cref<math::vec2> outer_,
		cref<math::vec2> size_
	) {

		math::vec2 size = size_;

		/**/

		if (attributes_.minWidth->type == ReflowUnitType::eRelative) {
			size.x = MAX(size.x, attributes_.minWidth->value * outer_.x);
		} else if (attributes_.minWidth->type == ReflowUnitType::eAbsolute) {
			size.x = MAX(size.x, attributes_.minWidth->value);
		}

		if (attributes_.minHeight->type == ReflowUnitType::eRelative) {
			size.y = MAX(size.y, attributes_.minHeight->value * outer_.y);
		} else if (attributes_.minHeight->type == ReflowUnitType::eAbsolute) {
			size.y = MAX(size.y, attributes_.minHeight->value);
		}

		/**/

		if (attributes_.maxWidth->type == ReflowUnitType::eRelative) {
			size.x = MIN(size.x, attributes_.maxWidth->value * outer_.x);
		} else if (attributes_.maxWidth->type == ReflowUnitType::eAbsolute) {
			size.x = MIN(size.x, attributes_.maxWidth->value);
		}

		if (attributes_.maxHeight->type == ReflowUnitType::eRelative) {
			size.y = MIN(size.y, attributes_.maxHeight->value * outer_.y);
		} else if (attributes_.maxHeight->type == ReflowUnitType::eAbsolute) {
			size.y = MIN(size.y, attributes_.maxHeight->value);
		}

		/**/

		return size;
	}

	template <>
	[[nodiscard]] inline math::vec2 clampSize<BoxLayoutAttributes>(
		cref<BoxLayoutAttributes> box_,
		cref<math::vec2> outer_,
		cref<math::vec2> size_
	) {

		math::vec2 size = size_;

		/**/

		if (box_.valueOf<attr::BoxLayout::minWidth>().type == ReflowUnitType::eRelative) {
			size.x = MAX(size.x, box_.valueOf<attr::BoxLayout::minWidth>().value * outer_.x);
		} else if (box_.valueOf<attr::BoxLayout::minWidth>().type == ReflowUnitType::eAbsolute) {
			size.x = MAX(size.x, box_.valueOf<attr::BoxLayout::minWidth>().value);
		}

		if (box_.valueOf<attr::BoxLayout::minHeight>().type == ReflowUnitType::eRelative) {
			size.y = MAX(size.y, box_.valueOf<attr::BoxLayout::minHeight>().value * outer_.y);
		} else if (box_.valueOf<attr::BoxLayout::minHeight>().type == ReflowUnitType::eAbsolute) {
			size.y = MAX(size.y, box_.valueOf<attr::BoxLayout::minHeight>().value);
		}

		/**/

		if (box_.valueOf<attr::BoxLayout::maxWidth>().type == ReflowUnitType::eRelative) {
			size.x = MIN(size.x, box_.valueOf<attr::BoxLayout::maxWidth>().value * outer_.x);
		} else if (box_.valueOf<attr::BoxLayout::maxWidth>().type == ReflowUnitType::eAbsolute) {
			size.x = MIN(size.x, box_.valueOf<attr::BoxLayout::maxWidth>().value);
		}

		if (box_.valueOf<attr::BoxLayout::maxHeight>().type == ReflowUnitType::eRelative) {
			size.y = MIN(size.y, box_.valueOf<attr::BoxLayout::maxHeight>().value * outer_.y);
		} else if (box_.valueOf<attr::BoxLayout::maxHeight>().type == ReflowUnitType::eAbsolute) {
			size.y = MIN(size.y, box_.valueOf<attr::BoxLayout::maxHeight>().value);
		}

		/**/

		return size;
	}
}
