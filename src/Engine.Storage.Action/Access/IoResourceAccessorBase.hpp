#pragma once

#include <Engine.Common/Wrapper.hpp>

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
