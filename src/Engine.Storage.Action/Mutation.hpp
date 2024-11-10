#pragma once

#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Reflect.Utils/With/WithType.hpp>
#include <Engine.Storage.Registry/IStorage.hpp>

#include "Action.hpp"
#include "Context.hpp"

namespace hg::engine::storage {
	class IoResourceAccessorBase;
}

namespace hg::engine::storage::system {
	template <typename...>
	class MutationStaticGen;

	class MutationDynamicGen;
}

namespace hg::engine::storage {
	/**/

	class Mutation :
		public Action {
	public:
		template <typename...>
		friend class system::MutationStaticGen;

		friend class system::MutationDynamicGen;

	public:
		using this_type = Mutation;

		using exec_noreturn_type = void;
		using exec_valreturn_type = meta::WithType<UniquePtr<void, void(*)(void*)>>;

		using Action::pre_error_type;
		using Action::acq_error_type;

	protected:
		constexpr Mutation() noexcept = default;

	public:
		constexpr virtual ~Mutation() noexcept = default;

	public:
		decltype(auto) exec(auto&& fn_) noexcept {
			// TODO: We need to query/reconstruct the call parameters for fn, as we need the type for static/dynamic type validation.
			// TODO: return std::invoke(std::forward<decltype(fn_)>(fn_), access.stage2);
			std::unreachable();
		}

		virtual exec_noreturn_type exec(
			_Inout_ mref<std::function<exec_noreturn_type(cref<IoResourceAccessorBase>)>>
		) = 0;

		virtual exec_valreturn_type exec(
			_Inout_ mref<std::function<exec_valreturn_type(cref<IoResourceAccessorBase>)>>
		) = 0;

	public:
		[[nodiscard]] virtual Result<void, pre_error_type> prepare(_Inout_ mref<Arci<IStorage>> storage_) = 0;

		[[nodiscard]] virtual Result<void, acq_error_type> acquire(_Inout_ ref<Context> ctx_) = 0;

		virtual void release(_Inout_ ref<Context> ctx_) = 0;

		virtual void cleanup() = 0;
	};
}
