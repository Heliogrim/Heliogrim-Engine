#pragma once

#include "../Wrapper.hpp"
#include "Vector.hpp"

namespace hg::math {
	class Location final :
		protected fvec3 {
	public:
		using this_type = Location;
		using math_type = fvec3;

		using value_type = math_type::value_type;

	public:
		Location() noexcept;

		Location(const value_type x_, const value_type y_, const value_type z_) noexcept;

		explicit Location(cref<math_type> location_) noexcept;

		Location(mref<Location> other_) noexcept;

		Location(cref<Location> other_) noexcept;

		~Location() noexcept = default;

	public:
		ref<this_type> operator=(mref<this_type> other_) noexcept;

		ref<this_type> operator=(cref<this_type> other_) noexcept;

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
