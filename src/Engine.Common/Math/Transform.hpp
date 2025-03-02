#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "Location.hpp"
#include "Rotator.hpp"

namespace hg::math {
	class Transform {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = Transform;

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

		Transform(mref<Location> location_, mref<Rotator> rotator_, mref<fvec3> scale_) noexcept;

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
		fvec3 _scale;

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

		[[nodiscard]] cref<fvec3> scale() const noexcept;

		[[nodiscard]] ref<fvec3> scale() noexcept;

		ref<this_type> setScale(mref<fvec3> scale_) noexcept;
	};
}
