#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "AwaitSignal.hpp"

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
}
