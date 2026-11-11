#pragma once

#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Storage.Registry/IStorage.hpp>

#include "DynamicStage.hpp"
#include "../Query.hpp"
#include "../Access/IoResourceAccessor.hpp"

namespace hg::engine::storage::system {
	class QueryDynamicGen final :
		public Query {
	public:
		using this_type = QueryDynamicGen;

		using index_type = std::size_t;

	public:
		QueryDynamicGen() = delete;

		explicit QueryDynamicGen(_Inout_ mref<InlineAutoArray<DynamicStage, 3>> stages_) noexcept;

		QueryDynamicGen(mref<this_type>) noexcept = default;

		~QueryDynamicGen() noexcept override = default;

	private:
		InlineAutoArray<DynamicStage, 3> _stages;

	private:
		[[nodiscard]] const auto& getFirstStage() const noexcept {
			return _stages.front();
		}

		[[nodiscard]] auto& getFirstStage() noexcept {
			return _stages.front();
		}

	public:
		decltype(auto) exec(auto&& fn_) noexcept {
			return std::invoke(std::forward<decltype(fn_)>(fn_), *_top->access.value);
		}

		exec_noreturn_type exec(mref<std::function<exec_noreturn_type(cref<IoResourceAccessorBase>)>> fn_) override {
			std::invoke(::hg::forward<decltype(fn_)>(fn_), *_top->access.value);
		}

		exec_valreturn_type exec(mref<std::function<exec_valreturn_type(cref<IoResourceAccessorBase>)>> fn_) override {
			return std::invoke(::hg::forward<decltype(fn_)>(fn_), *_top->access.value);
		}

	public:
		[[nodiscard]] Result<void, pre_error_type> prepare(mref<Arci<IStorage>> storage_) override {

			getFirstStage().storage = ::hg::move(storage_);
			for (index_type stageIdx = 1; stageIdx < _stages.size(); ++stageIdx) {

				const auto& prev = (_stages.cbegin() + stageIdx - 1uLL)->storage;
				auto& cur = (_stages.begin() + stageIdx)->storage;

				if (prev->isPrimaryStorage()) {
					// TODO: Semantically a IllegalArgumentError ...
					return Unexpected<pre_error_type> { IllegalTypeError { "Encountered primary storage while populating query stack." } };
				}

				cur = clone(prev).into<ISecondaryStorage>()->getBacking();
			}

			if (not _stages.back().storage->isPrimaryStorage()) {
				// TODO: Semantically a IllegalArgumentError ...
				return Unexpected<pre_error_type> { IllegalTypeError { "Encountered secondary storage while concluding query stack." } };
			}

			_primary = _stages.back().storage.get();
			return {};
		}

		[[nodiscard]] Result<void, acq_error_type> acquire(ref<Context> ctx_) override {

			// Note: Scan from the top down for the highest cached stage; everything below it is shared.
			const auto count = _stages.size();
			auto hitIdx = count;
			Arci<ResolvedStage> below {};

			for (index_type stageIdx = 0; stageIdx < count; ++stageIdx) {

				const auto& stage = *(_stages.begin() + stageIdx);
				auto found = ctx_.stacks.find(StageKey { .storage = stage.storage.get(), .resource = stage.resourceType });

				if (found.has_value()) {
					below = ::hg::move(found).value();
					hitIdx = stageIdx;
					break;
				}
			}

			/**/

			for (auto stageIdx = hitIdx; stageIdx-- > 0;) {

				const auto& stage = *(_stages.begin() + stageIdx);
				auto result = stage.acqFn(
					ctx_,
					stage.storage,
					below != nullptr ? below->access : meta::WithType<Rc<IoResourceAccessorBase>> {}
				);

				if (not result.has_value()) {
					return Unexpected { ::hg::move(result).error() };
				}

				auto node = Arci<ResolvedStage>::create();
				node->key = StageKey { stage.storage.get(), stage.resourceType };
				node->storage = clone(stage.storage);
				node->access = ::hg::move(result).value();
				node->primary = below != nullptr ? below->primary : stage.storage.get();
				node->shared = stage.shareable && (below == nullptr || below->shared);
				node->below = ::hg::move(below);

				// Note: First insert wins; a concurrent reader may have resolved the same stage.
				below = node->shared ? ctx_.stacks.insert(::hg::move(node)) : ::hg::move(node);
			}

			_top = ::hg::move(below);
			return {};
		}

		void release([[maybe_unused]] ref<Context> ctx_) override {
			_top.reset();
		}

		void cleanup() override {
			for (auto& stage : _stages) {
				::hg::assertd(stage.prepared());
				stage.storage.reset();
			}
		}
	};
}
