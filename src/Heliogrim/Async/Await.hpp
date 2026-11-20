#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Waitable.hpp"

namespace hg {
	extern void await(_Inout_ ref<Signal> signal_);

	extern void await(_Inout_ ref<JoinCounter> counter_);

	/**
	 * Blocks without bound until every index has finished; asserts on an affine thread. batch_ must not be moved-from.
	 */
	extern void await(_Inout_ ref<Batch> batch_);
}
