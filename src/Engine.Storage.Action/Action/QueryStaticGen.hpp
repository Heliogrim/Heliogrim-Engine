#pragma once

#include <type_traits>
#include <utility>
#include <Engine.Reflect/FullTypeInfo.hpp>

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
		inline constexpr static index_type stage_count = sizeof...(Stages_);

		template <index_type Idx_>
		using stage_resource_type = typename std::remove_cvref_t<
			decltype(std::get<Idx_>(std::declval<std::tuple<Stages_...>&>()).access)
		>::resource_type;

		template <index_type Idx_>
		[[nodiscard]] static ref<const IoResourceAccessor<stage_resource_type<Idx_>>> typedAccess(
			ref<const Arci<ResolvedStage>> node_
		) noexcept {
			::hg::assertd(node_->access.typeId == ::hg::refl::TypeId<Rc<IoResourceAccessor<stage_resource_type<Idx_>>>>());
			return static_cast<ref<const IoResourceAccessor<stage_resource_type<Idx_>>>>(*node_->access.value);
		}

		template <index_type Idx_>
		[[nodiscard]] StageKey stageKey() const noexcept {
			return StageKey {
				.storage = std::get<Idx_>(_stages).storage.get(),
				// Note: Keyed by the exact accessor type, as `typedAccess` downcasts a found node to it.
				.resource = ::hg::refl::TypeId<Rc<IoResourceAccessor<stage_resource_type<Idx_>>>>()
			};
		}

		template <index_type Idx_>
		[[nodiscard]] bool findStage(ref<Context> ctx_, ref<index_type> hitIdx_, ref<Arci<ResolvedStage>> hit_) const {

			auto found = ctx_.stacks.find(stageKey<Idx_>());
			if (not found.has_value()) {
				return false;
			}

			hitIdx_ = Idx_;
			hit_ = ::hg::move(found).value();
			return true;
		}

		template <index_type Idx_>
		[[nodiscard]] Result<void, acq_error_type> buildStage(ref<Context> ctx_, ref<Arci<ResolvedStage>> below_) {
			auto& stage = std::get<Idx_>(_stages);

			auto expect = [&] {
				if constexpr (Idx_ + 1uLL == stage_count) {
					return stage.acqFn(ctx_, stage.storage);

				} else {
					return stage.acqFn(ctx_, stage.storage, typedAccess<Idx_ + 1uLL>(below_));
				}
			}();

			if (not expect.has_value()) {
				return Unexpected<acq_error_type>(::hg::move(expect).error());
			}

			auto node = Arci<ResolvedStage>::create();
			node->key = stageKey<Idx_>();
			node->storage = clone(stage.storage).template into<IStorage>();
			node->access = meta::WithType<Rc<IoResourceAccessorBase>>::from(
				Rc<IoResourceAccessor<stage_resource_type<Idx_>>>::create(::hg::move(expect).value())
			);
			node->primary = below_ != nullptr ? below_->primary : stage.storage.get();
			node->below = ::hg::move(below_);

			// Note: First insert wins; a concurrent reader may have resolved the same stage.
			below_ = ctx_.stacks.insert(::hg::move(node));
			return {};
		}

	public:
		decltype(auto) exec(auto&& fn_) noexcept {
			return std::invoke(std::forward<decltype(fn_)>(fn_), typedAccess<0>(_top));
		}

		exec_noreturn_type exec(mref<std::function<exec_noreturn_type(cref<IoResourceAccessorBase>)>> fn_) override {
			std::invoke(std::forward<decltype(fn_)>(fn_), *_top->access.value);
		}

		exec_valreturn_type exec(mref<std::function<exec_valreturn_type(cref<IoResourceAccessorBase>)>> fn_) override {
			return std::invoke(std::forward<decltype(fn_)>(fn_), *_top->access.value);
		}

	private:
		decltype(auto) prepareTypeOrdered(auto&& storage_) {
			constexpr auto crtFn = PreInvOrdered<Stages_...> {};
			return crtFn(_stages, ::hg::forward<decltype(storage_)>(storage_));
		}

	public:
		[[nodiscard]] Result<void, pre_error_type> prepare(_Inout_ mref<Arci<IStorage>> storage_) override {
			auto result = prepareTypeOrdered(::hg::forward<decltype(storage_)>(storage_));
			if (result.has_value()) {
				_primary = std::get<stage_count - 1uLL>(_stages).storage.get();
			}
			return result;
		}

	private:
		template <index_type... Idx_>
		[[nodiscard]] Result<void, acq_error_type> acquireIndexed(ref<Context> ctx_, std::index_sequence<Idx_...>) {

			// Note: Scan from the top down for the highest cached stage; everything below it is shared.
			auto hitIdx = stage_count;
			Arci<ResolvedStage> below {};
			static_cast<void>((findStage<Idx_>(ctx_, hitIdx, below) || ...));

			// Note: Build the missing stages bottom-up; stop at the first failure without inserting above it.
			Result<void, acq_error_type> result {};
			static_cast<void>(
				(
					(stage_count - 1uLL - Idx_ >= hitIdx || (
						result = buildStage<stage_count - 1uLL - Idx_>(ctx_, below), result.has_value()
					)) && ...
				)
			);
			if (not result.has_value()) {
				return result;
			}

			_top = ::hg::move(below);
			return {};
		}

		decltype(auto) cleanupTypeOrdered() {
			constexpr auto dstFn = ClnInvOrdered<Stages_...> {};
			return dstFn(_stages);
		}

	public:
		[[nodiscard]] Result<void, acq_error_type> acquire(_Inout_ ref<Context> ctx_) override {
			return acquireIndexed(ctx_, std::make_index_sequence<stage_count> {});
		}

		void release([[maybe_unused]] _Inout_ ref<Context> ctx_) override {
			_top.reset();
		}

		void cleanup() override {
			cleanupTypeOrdered();
		}
	};
}
