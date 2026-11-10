#pragma once

#include <type_traits>
#include <utility>
#include <Engine.Core/Engine.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Task/Task.hpp>

#include "FutureState.hpp"

namespace hg::detail {
	template <typename T_, typename E_>
	void scheduleContinuation(mref<Arci<FutureState<T_, E_>>> state_) {
		const auto mask = state_->mask;
		::hg::engine::Engine::getEngine()->getScheduler()->exec(
			::hg::engine::scheduler::task::make_task(
				[state = std::move(state_)] {
					std::move(state->cont)(*state.get());
				},
				mask
			)
		);
	}

	/**
	 * Publishes a filled slot and hands a stored continuation to the scheduler, never running it inline.
	 */
	template <typename T_, typename E_>
	void complete(ref<const Arci<FutureState<T_, E_>>> state_) {
		state_->done.set();
		if (state_->contState.exchange(eClaimed, std::memory_order::acq_rel) == eStored) {
			scheduleContinuation(Arci<FutureState<T_, E_>> { state_ });
		}
	}

	template <typename T_, typename E_>
	void store(
		ref<const Arci<FutureState<T_, E_>>> state_,
		std::type_identity_t<UniqueFn<void(ref<FutureState<T_, E_>>)>>&& cont_
	) {
		state_->cont = std::move(cont_);
		if (state_->contState.exchange(eStored, std::memory_order::acq_rel) == eClaimed) {
			scheduleContinuation(Arci<FutureState<T_, E_>> { state_ });
		}
	}

	template <typename E_>
	[[nodiscard]] Fail<E_> makeFail(mref<E_> error_) {
		if constexpr (std::is_same_v<E_, Broken>) {
			return std::move(error_);
		} else {
			return Fail<E_> { std::in_place_type<E_>, std::move(error_) };
		}
	}

	template <typename To_, typename From_>
	[[nodiscard]] Fail<To_> convertFail(mref<Fail<From_>> error_) {
		if constexpr (std::is_same_v<From_, Broken>) {
			if constexpr (std::is_same_v<To_, Broken>) {
				return std::move(error_);
			} else {
				return Fail<To_> { std::in_place_type<Broken>, std::move(error_) };
			}

		} else if constexpr (std::is_same_v<To_, From_>) {
			return std::move(error_);

		} else {
			if (error_.template is<Broken>()) {
				if constexpr (std::is_same_v<To_, Broken>) {
					return error_.template as<Broken>();
				} else {
					return Fail<To_> { std::in_place_type<Broken>, error_.template as<Broken>() };
				}
			}
			return makeFail<To_>(To_(std::move(error_).template as<From_>()));
		}
	}
}
