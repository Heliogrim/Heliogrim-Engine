#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::storage {
	class IoResourceAccessorBase {
	public:
		using this_type = IoResourceAccessorBase;

	protected:
		constexpr IoResourceAccessorBase() noexcept = default;

		constexpr IoResourceAccessorBase(IoResourceAccessorBase&&) noexcept = default;

	public:
		IoResourceAccessorBase(const IoResourceAccessorBase&) = delete;

		constexpr virtual ~IoResourceAccessorBase() noexcept = default;

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

	protected:
		constexpr ref<this_type> operator=(mref<this_type>) noexcept = default;
	};
}
