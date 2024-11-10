#pragma once

#include <Engine.Common/Move.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg::engine::resource {
	class Blob;
}

namespace hg::engine::resource {
	class Package :
		public InheritBase<Package> {
	public:
		using this_type = Package;

	protected:
		constexpr Package() noexcept = default;

		constexpr Package(mref<this_type> other_) noexcept :
			InheritBase(::hg::move(other_)) {}

		constexpr Package(cref<this_type> other_) noexcept :
			InheritBase(other_) {}

		constexpr ~Package() noexcept override = default;
	};
}
