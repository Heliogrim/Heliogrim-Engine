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
	class QueryStaticGen;

	class QueryDynamicGen;
}

namespace hg::engine::storage {
	/**/

	class Query :
		public Action {
	public:
		template <typename...>
		friend class system::QueryStaticGen;

		friend class system::QueryDynamicGen;

	public:
		using this_type = Query;

		using exec_noreturn_type = void;
		using exec_valreturn_type = meta::WithType<UniquePtr<void, void(*)(void*)>>;

		using Action::pre_error_type;
		using Action::acq_error_type;

	protected:
		constexpr Query() noexcept = default;

	public:
		constexpr virtual ~Query() noexcept = default;

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
