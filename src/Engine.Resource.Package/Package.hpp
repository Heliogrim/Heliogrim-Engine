#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
