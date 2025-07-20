#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "Location.hpp"
#include "Matrix.hpp"
#include "Rotator.hpp"

namespace hg::math {
	class Transform {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = Transform;
		using scale_type = Vector3f32;

	public:
		/**
		 * Default Constructor
		 *
		 * @author Julius
		 * @date 26.11.2021
		 */
		constexpr Transform() noexcept :
			_location(),
			_rotator(),
			_scale(1.F) {}

		Transform(mref<Location> location_, mref<Rotator> rotator_, mref<scale_type> scale_) noexcept;

		/**
		 * Move Constructor
		 *
		 * @author Julius
		 * @date 26.11.2021
		 *
		 * @param other_ The transform to move from.
		 */
		Transform(mref<Transform> other_) noexcept = default;

		/**
		 * Copy Constructor
		 *
		 * @author Julius
		 * @date 26.11.2021
		 *
		 * @param other_ The transform to copy from.
		 */
		Transform(cref<Transform> other_) = default;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 26.11.2021
		 */
		~Transform() = default;

	public:
		/**
		 * Move Assignment
		 *
		 * @author Julius
		 * @date 02.12.2021
		 *
		 * @param other_ The transform to move from.
		 *
		 * @returns A shallow copy of this.
		 */
		ref<Transform> operator=(mref<Transform> other_) noexcept;

		/**
		 * Copy Assignment
		 *
		 * @author Julius
		 * @date 02.12.2021
		 *
		 * @param other_ The transform to copy from.
		 *
		 * @returns A shallow copy of this.
		 */
		ref<Transform> operator=(cref<Transform> other_) noexcept;

	private:
	public:
		Location _location;
		Rotator _rotator;
		scale_type _scale;

	public:
		[[nodiscard]] cref<Location> location() const noexcept;

		[[nodiscard]] ref<Location> location() noexcept;

		ref<this_type> setLocation(mref<Location> location_) noexcept;

		template <typename... Args_> requires std::is_constructible_v<Location, Args_...>
		ref<this_type> setLocation(Args_&&... args_) noexcept {
			return setLocation({ std::forward<Args_>(args_)... });
		}

		[[nodiscard]] cref<Rotator> rotator() const noexcept;

		[[nodiscard]] ref<Rotator> rotator() noexcept;

		ref<this_type> setRotator(mref<Rotator> rotator_) noexcept;

		[[nodiscard]] cref<scale_type> scale() const noexcept;

		[[nodiscard]] ref<scale_type> scale() noexcept;

		ref<this_type> setScale(mref<scale_type> scale_) noexcept;

	public:
		[[nodiscard]] math::mat4 asMatrix() const noexcept;

		friend math::mat4 operator*(ref<const Transform> outer_, ref<const Transform> inner_) noexcept;

		friend math::mat4 operator*(ref<const math::mat4> outer_, ref<const Transform> inner_) noexcept;
	};
}
