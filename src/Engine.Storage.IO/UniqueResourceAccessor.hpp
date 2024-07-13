#pragma once

#include <Engine.Common/UniqueValue.hpp>
#include <Engine.Pedantic.Resource/ResourceAccessor.hpp>

namespace hg::engine::storage {
	template <typename Type_>
	class UniqueResourceAccessor final :
		public UniqueValue<ResourceAccessor<Type_>, std::function<void(const ptr<void>)>> {
	public:
		using this_type = UniqueResourceAccessor<Type_>;
		using report_fn_type = std::function<void(const ptr<void>)>;

	public:
		using UniqueValue<ResourceAccessor<Type_>, report_fn_type>::UniqueValue;

	public:
		// TODO: Check whether we want to hoist the complete interface of the resource accessor into this class.
		[[nodiscard]] constexpr bool invalid() const noexcept {
			return this->get().invalid();
		}

		[[nodiscard]] auto* operator->() const noexcept {
			return std::addressof(***this);
		}
	};
}
