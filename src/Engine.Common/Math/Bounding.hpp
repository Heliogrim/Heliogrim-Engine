#pragma once

#include "Vector.hpp"
#include "../Sal.hpp"
#include "../Wrapper.hpp"

namespace hg::math {
	class Bounding {
	public:
		using this_type = Bounding;

	public:
		Bounding() noexcept;

		constexpr Bounding(_In_ cref<vec3> center_, _In_ cref<vec3> extent_) noexcept :
			_center(center_),
			_extent(extent_),
			_extent_length() {}

		constexpr Bounding(_In_ cref<vec3> center_, _In_ cref<vec3> extent_, _In_ vec3::value_type length_) noexcept :
			_center(center_),
			_extent(extent_),
			_extent_length(length_) {}

		constexpr Bounding(cref<this_type> other_) noexcept = default;

		constexpr Bounding(mref<this_type> other_) noexcept = default;

		constexpr ~Bounding() = default;

	public:
		ref<this_type> operator=(mref<this_type> other_) noexcept = default;

		ref<this_type> operator=(cref<this_type> other_) noexcept = default;

	public:
		/**
		 * Checking Functions
		 */

		/**
		 * Check if point intersects spherical
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @param  position_ The position.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] bool intersectsSpherical(cref<vec3> position_) const noexcept;

		/**
		 * Check if spherical volume intersects spherical
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @param  position_ The position.
		 * @param  radius_ The radius.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] bool intersectsSpherical(cref<vec3> position_, const float radius_) const noexcept;

		/**
		 * Check if other Bounding intersects spherical
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @param  other_ The other.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] bool intersectsSpherical(cref<Bounding> other_) const noexcept;

		/**
		 * Check if point intersects axis-aligned bounding box
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @param  position_ The position.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] bool intersectsAaBb(cref<vec3> position_) const noexcept;

		/**
		 * Check if point with volumetric extent intersects axis-aligned bounding box
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @param  position_ The position.
		 * @param  extent_ The extent.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] bool intersectsAaBb(cref<vec3> position_, cref<vec3> extent_) const noexcept;

		/**
		 * Check if other Bounding intersects axis-aligned bounding box
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @param  other_ The other.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] bool intersectsAaBb(cref<Bounding> other_) const noexcept;

		/**
		 * Check if Bounding contains position with spherical extent fully by spherical volume
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @param  position_ The position.
		 * @param  radius_ The radius.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] bool containsSpherical(cref<vec3> position_, cref<vec3::value_type> radius_) const noexcept;

		/**
		 * Check if Bounding contains position with extent fully by aabb
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @param  position_ The position.
		 * @param  extent_ The extent.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] bool containsAaBb(cref<vec3> position_, cref<vec3> extent_) const noexcept;

	private:
	public:
		/**
		 * Center
		 */
		vec3 _center;

	public:
		/**
		 * Gets a immutable reference to the center
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @returns A cref&lt;vec3&gt;
		 */
		[[nodiscard]] cref<vec3> center() const noexcept;

		/**
		 * Gets a mutable reference to the center
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @returns A ref&lt;vec3&gt;
		 */
		[[nodiscard]] ref<vec3> center() noexcept;

	private:
	public:
		/**
		 * Extent
		 */
		vec3 _extent;
		vec3::value_type _extent_length;

		/**
		 * Restore length
		 *
		 * @author Julius
		 * @date 27.03.2021
		 */
		void restoreLength();

	public:
		/**
		 * Gets a immutable reference to the extent
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @returns A cref&lt;vec3&gt;
		 */
		[[nodiscard]] cref<vec3> extent() const noexcept;

		/**
		 * Gets a immutable reference to the length
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @returns A cref&lt;vec3::value_type&gt;
		 */
		[[nodiscard]] cref<vec3::value_type> extentLength() const noexcept;

		/**
		 * Sets the stored extent and recalculates it's length
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @param  extent_ The extent.
		 */
		void setExtent(cref<vec3> extent_);

		/**
		 * Sets the stored extent and recalculate it's length
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @param  a_ a.
		 * @param  b_ The b.
		 */
		void setExtent(cref<vec3::value_type> a_, cref<vec3::value_type> b_);

		/**
		 * Sets an extent
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @param  x_ The x coordinate.
		 * @param  y_ The y coordinate.
		 * @param  z_ The z coordinate.
		 */
		void setExtent(cref<vec3::value_type> x_, cref<vec3::value_type> y_, cref<vec3::value_type> z_);

		/**
		 * Applies the extent scale to the stored data
		 *
		 * @author Julius
		 * @date 27.03.2021
		 *
		 * @param  scale_ The scale.
		 */
		void applyExtentScale(cref<vec3::value_type> scale_);
	};
}
