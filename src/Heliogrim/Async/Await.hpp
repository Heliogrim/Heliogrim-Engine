#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "AwaitSignal.hpp"
#include "Waitable.hpp"

namespace hg {
	/**
	 * Awaits the current execution context until signal arise
	 *
	 * @author Julius
	 * @date 23.10.2021
	 *
	 * @param  signal_ The signal where to subscribe / wait for.
	 */
	[[deprecated("await(ref<Signal>) replaces atomic-flag signals")]]
	extern void await(_In_ ptr<await_signal_sub_type> signal_);

	extern void await(_Inout_ ref<Signal> signal_);

	extern void await(_Inout_ ref<JoinCounter> counter_);
}
