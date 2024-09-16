#pragma once

#include "Vector.hpp"
#include "../Wrapper.hpp"

namespace hg::math {
	class Location final :
		protected fvec3 {
	public:
		using this_type = Location;
		using math_type = fvec3;

		using value_type = math_type::value_type;

	public:
		constexpr Location() noexcept = default;

		Location(const value_type x_, const value_type y_, const value_type z_) noexcept;

		explicit Location(cref<math_type> location_) noexcept;

		constexpr Location(mref<Location> other_) noexcept = default;

		constexpr Location(cref<Location> other_) noexcept = default;

		constexpr ~Location() noexcept = default;

	public:
		constexpr ref<this_type> operator=(mref<this_type> other_) noexcept = default;

		constexpr ref<this_type> operator=(cref<this_type> other_) noexcept = default;

	public:
		void setX(const value_type x_) noexcept;

		void setY(const value_type y_) noexcept;

		void setZ(const value_type z_) noexcept;

	public:
		[[nodiscard]] bool operator==(cref<this_type> other_) const noexcept;

		[[nodiscard]] bool operator!=(cref<this_type> other_) const noexcept;

	public:
		cref<fvec3> into() const noexcept;
	};
}
