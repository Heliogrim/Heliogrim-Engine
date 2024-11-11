#pragma once

#include <type_traits>
#include <utility>

#include "StaticGen.hpp"
#include "StaticStage.hpp"
#include "../Context.hpp"
#include "../Query.hpp"

namespace hg::engine::storage::system {
	template <typename... Stages_>
	class QueryStaticGen final :
		public Query {
	public:
		using this_type = QueryStaticGen<Stages_...>;

		using index_type = std::size_t;

	public:
		~QueryStaticGen() override = default;

	private:
		std::tuple<Stages_...> _stages;

	private:
		[[nodiscard]] const auto& getFirstStage() const noexcept {
			return std::get<0>(_stages);
		}

	public:
		decltype(auto) exec(auto&& fn_) noexcept {
			return std::invoke(std::forward<decltype(fn_)>(fn_), getFirstStage().access);
		}

		exec_noreturn_type exec(mref<std::function<exec_noreturn_type(cref<IoResourceAccessorBase>)>> fn_) override {
			std::invoke(std::forward<decltype(fn_)>(fn_), getFirstStage().access);
		}

		exec_valreturn_type exec(mref<std::function<exec_valreturn_type(cref<IoResourceAccessorBase>)>> fn_) override {
			return std::invoke(std::forward<decltype(fn_)>(fn_), getFirstStage().access);
		}

	private:
		decltype(auto) prepareTypeOrdered(auto&& storage_) {
			constexpr auto crtFn = PreInvOrdered<Stages_...> {};
			return crtFn(_stages, ::hg::forward<decltype(storage_)>(storage_));
		}

	public:
		[[nodiscard]] Result<void, pre_error_type> prepare(_Inout_ mref<Arci<IStorage>> storage_) override {
			return prepareTypeOrdered(::hg::forward<decltype(storage_)>(storage_));
		}

	private:
		decltype(auto) acquireTypeOrdered(_Inout_ ref<Context> ctx_) {
			constexpr auto acqFn = AcqInvOrdered<Stages_...> {};
			return acqFn(_stages, ctx_);
		}

	public:
		[[nodiscard]] Result<void, acq_error_type> acquire(_Inout_ ref<Context> ctx_) override {
			return acquireTypeOrdered(ctx_);
		}

	private:
		decltype(auto) releaseTypeOrdered(_Inout_ ref<Context> ctx_) {
			constexpr auto crtFn = RelInvOrdered<Stages_...> {};
			return crtFn(_stages, ::hg::forward<decltype(ctx_)>(ctx_));
		}

	public:
		void release(_Inout_ ref<Context> ctx_) override {
			releaseTypeOrdered(ctx_);
		}

	private:
		decltype(auto) cleanupTypeOrdered() {
			constexpr auto dstFn = ClnInvOrdered<Stages_...> {};
			return dstFn(_stages);
		}

	public:
		void cleanup() override {
			cleanupTypeOrdered();
		}
	};
}
