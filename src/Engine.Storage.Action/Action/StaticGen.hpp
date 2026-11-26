#pragma once

#include <Engine.Common/Sal.hpp>

#include "../Action.hpp"
#include "../Context.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Asserts;
import Engine.Common;
import Engine.IO.Dispatcher;
import Engine.Reflect;
#endif

namespace hg::engine::storage::system {
	template <typename Type_, typename... Stages_>
	struct PreInvOrdered;

	template <typename Type_>
	struct PreInvOrdered<Type_> {
		Result<void, Action::pre_error_type> operator()(
			_Inout_ auto& stages_,
			auto&& storage_
		) const noexcept {

			using target_storage_type = typename decltype(Type_::storage)::value_type;
			::hg::assertd(IsType<target_storage_type>(*storage_));

			if (not IsType<target_storage_type>(*storage_)) {
				return Unexpected<Action::pre_error_type> {
					IllegalTypeError { "Encountered unexpected type while preparing static stage." }
				};
			}

			/**/

			auto& stage = std::get<Type_>(stages_);
			stage.storage = ::hg::move(storage_).template into<target_storage_type>();

			return {};
		}
	};

	template <typename Type_, typename Next_, typename... Stages_>
	struct PreInvOrdered<Type_, Next_, Stages_...> {
		Result<void, Action::pre_error_type> operator()(
			_Inout_ auto& stages_,
			auto&& storage_
		) const noexcept {

			using target_storage_type = typename decltype(Type_::storage)::value_type;
			::hg::assertd(IsType<target_storage_type>(*storage_));

			if (not IsType<target_storage_type>(*storage_)) {
				return Unexpected<Action::pre_error_type> {
					IllegalTypeError { "Encountered unexpected type while preparing static stage." }
				};
			}

			/**/

			auto& stage = std::get<Type_>(stages_);
			stage.storage = ::hg::move(storage_).template into<target_storage_type>();

			/**/

			constexpr auto nextFn = PreInvOrdered<Next_, Stages_...> {};
			return nextFn(stages_, stage.storage->getBacking());
		}
	};

	/**/

	template <typename Type_, typename... Stages_>
	struct AcqInvOrdered;

	template <typename Type_>
	struct AcqInvOrdered<Type_> {
		Result<void, Action::acq_error_type> operator()(
			_Inout_ auto& stages_,
			_Inout_ ref<Context> ctx_
		) const noexcept {

			auto& stage = std::get<Type_>(stages_);
			auto expect = stage.acqFn(ctx_, stage.storage);
			if (not expect.has_value()) {
				return Unexpected<Action::acq_error_type>(::hg::move(expect).error());
			}

			stage.access = ::hg::move(expect).value();
			return {};
		}
	};

	template <typename Type_, typename Next_, typename... Stages_>
	struct AcqInvOrdered<Type_, Next_, Stages_...> {
		Result<void, Action::acq_error_type> operator()(
			_Inout_ auto& stages_,
			_Inout_ ref<Context> ctx_
		) const noexcept {

			constexpr auto prevFn = AcqInvOrdered<Next_, Stages_...> {};
			auto prevResult = prevFn(stages_, ctx_);
			if (not prevResult.has_value()) {
				return ::hg::move(prevResult);
			}

			/**/

			const auto& prev = std::get<Next_>(stages_);
			auto& stage = std::get<Type_>(stages_);

			auto expect = stage.acqFn(ctx_, stage.storage, prev.access);
			if (not expect.has_value()) {
				return Unexpected<Action::acq_error_type>(::hg::move(expect).error());
			}

			stage.access = ::hg::move(expect).value();
			return {};
		}
	};

	/**/

	template <typename Type_, typename... Stages_>
	struct RelInvOrdered;

	/* A stage release either cannot fail (`void`) or reports a failed commit. */
	Result<void, io::CommitError> releaseStage(_Inout_ auto& stage_, _Inout_ ref<Context> ctx_) noexcept {
		if constexpr (std::is_void_v<decltype(stage_.relFn(ctx_, stage_.storage, ::hg::move(stage_.access)))>) {
			stage_.relFn(ctx_, stage_.storage, ::hg::move(stage_.access));
			return {};
		} else {
			return stage_.relFn(ctx_, stage_.storage, ::hg::move(stage_.access));
		}
	}

	template <typename Type_>
	struct RelInvOrdered<Type_> {
		Result<void, io::CommitError> operator()(
			_Inout_ auto& stages_,
			_Inout_ ref<Context> ctx_
		) const noexcept {
			return releaseStage(std::get<Type_>(stages_), ctx_);
		}
	};

	template <typename Type_, typename Next_, typename... Stages_>
	struct RelInvOrdered<Type_, Next_, Stages_...> {
		Result<void, io::CommitError> operator()(
			_Inout_ auto& stages_,
			_Inout_ ref<Context> ctx_
		) const noexcept {

			auto result = releaseStage(std::get<Type_>(stages_), ctx_);

			/**/

			// Note: Every stage is released even after a failure; the first error wins.
			constexpr auto nextFn = RelInvOrdered<Next_, Stages_...> {};
			auto next = nextFn(stages_, ctx_);
			return result.has_value() ? ::hg::move(next) : ::hg::move(result);
		}
	};

	/**/

	template <typename... Stages_>
	struct ClnInvOrdered;

	template <typename... Stages_>
	struct ClnInvOrdered {
		void operator()(_Inout_ auto& stages_) const noexcept {
			((std::get<Stages_>(stages_).storage.reset()), ...);
		}
	};
}
