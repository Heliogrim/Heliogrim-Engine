#pragma once

#include <Engine.Scheduler/Task/Batch.hpp>
#include <Engine.Scheduler/Waitable/JoinCounter.hpp>
#include <Engine.Scheduler/Waitable/Signal.hpp>

namespace hg {
	using Signal = engine::scheduler::Signal;
	using JoinCounter = engine::scheduler::JoinCounter;
	using Batch = engine::scheduler::Batch;
}
