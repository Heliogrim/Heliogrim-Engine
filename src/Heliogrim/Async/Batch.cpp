#include "Batch.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

hg::Batch hg::batch(const BatchCount count_, BatchFn fn_, const engine::scheduler::Lane lane_) {
	return engine::scheduler::batch(*engine::Engine::getEngine()->getScheduler(), count_, std::move(fn_), lane_);
}

void hg::parallelFor(const BatchCount count_, const BatchFnRef fn_, const engine::scheduler::Lane lane_) {
	engine::scheduler::parallelFor(*engine::Engine::getEngine()->getScheduler(), count_, fn_, lane_);
}
