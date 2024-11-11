#pragma once

#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Storage.Registry/IStorage.hpp>

#include "DynamicStage.hpp"
#include "../Query.hpp"
#include "../Access/AccessError.hpp"
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
			return std::invoke(std::forward<decltype(fn_)>(fn_), *getFirstStage().access.value);
		}

		exec_noreturn_type exec(mref<std::function<exec_noreturn_type(cref<IoResourceAccessorBase>)>> fn_) override {
			std::invoke(::hg::forward<decltype(fn_)>(fn_), *getFirstStage().access.value);
		}

		exec_valreturn_type exec(mref<std::function<exec_valreturn_type(cref<IoResourceAccessorBase>)>> fn_) override {
			return std::invoke(::hg::forward<decltype(fn_)>(fn_), *getFirstStage().access.value);
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

			return {};
		}

		[[nodiscard]] Result<void, acq_error_type> acquire(ref<Context> ctx_) override {

			/**/

			{
				auto& back = _stages.back();
				auto result = back.acqFn(ctx_, back.storage, {});

				if (not result.has_value()) {
					return Unexpected { ::hg::move(result).error() };
				}
				back.access = ::hg::move(result).value();

				if (_stages.size() <= 1) {
					return {};
				}
			}

			/**/

			const auto mark = --_stages.begin();
			for (auto iter = (_stages.end() -= 2); iter != mark; --iter) {

				const auto& prev = *(iter + 1);
				auto& cur = *iter;

				auto result = cur.acqFn(ctx_, cur.storage, prev.access);
				if (not result.has_value()) {
					return Unexpected { ::hg::move(result).error() };
				}

				cur.access = ::hg::move(result).value();
			}

			return {};
		}

		void release(ref<Context> ctx_) override {
			#if _DEBUG
			bool stride = false;
			for (auto& stage : _stages) {
				if (stage.active()) {
					stage.relFn(ctx_, stage.storage, ::hg::move(stage.access));
					stride = true;
					continue;
				}
				::hg::assertd(not stride);
			}
			#else
			for (auto& stage : _stages) {
				if (stage.active()) {
					stage.relFn(ctx_, stage.storage, ::hg::move(stage.access));
				}
			}
			#endif
		}

		void cleanup() override {
			for (auto& stage : _stages) {
				::hg::assertd(stage.prepared());
				stage.storage.reset();
			}
		}
	};
}
