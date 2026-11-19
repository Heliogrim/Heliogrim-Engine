#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg {
	template <typename ManagedType_>
	class macro_novtable ResourceAccessorBase {
	public:
		using this_type = ResourceAccessorBase<ManagedType_>;

		using managed_type = ManagedType_;

	protected:
		constexpr ResourceAccessorBase() noexcept = default;

		ResourceAccessorBase(ResourceAccessorBase&&) noexcept = default;

	public:
		ResourceAccessorBase(const ResourceAccessorBase&) = delete;

		constexpr virtual ~ResourceAccessorBase() noexcept = default;

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

	protected:
		ref<this_type> operator=(mref<this_type>) noexcept = default;
	};
}
