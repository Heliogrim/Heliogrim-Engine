#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Waitable.hpp"

namespace hg {
	extern void await(_Inout_ ref<Signal> signal_);

	extern void await(_Inout_ ref<JoinCounter> counter_);
}
