#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "AwaitSignal.hpp"
#include "Future.hpp"

namespace hg {
	/**
	 * Awaits the current execution context until signal arise
	 *
	 * @author Julius
	 * @date 23.10.2021
	 *
	 * @param  signal_ The signal where to subscribe / wait for.
	 */
	extern void await(_In_ ptr<await_signal_sub_type> signal_);

	/**
	 * Awaits the given future and returns the stored value
	 *
	 * @author Julius
	 * @date 26.11.2021
	 *
	 * @param future_ The future to wait for and resolved value from.
	 *
	 * @returns The stored value instance of the future.
	 */
	template <typename Type_>
	decltype(auto) await(_Inout_ mref<Future<Type_>> future_) {
		await(future_);
		return future_.get();
	}
}
