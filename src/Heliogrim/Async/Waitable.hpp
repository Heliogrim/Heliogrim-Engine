#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Scheduler;
#endif

namespace hg {
	using Signal = engine::scheduler::Signal;
	using JoinCounter = engine::scheduler::JoinCounter;
	using Batch = engine::scheduler::Batch;
}
