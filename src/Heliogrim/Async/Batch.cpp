#include "Batch.hpp"

import Engine.Core;
import Engine.Scheduler;


hg::Batch hg::batch(const BatchCount count_, BatchFn fn_, const engine::scheduler::Lane lane_) {
	return engine::scheduler::batch(*::hg::engine::Scheduler::get(), count_, std::move(fn_), lane_);
}

void hg::parallelFor(const BatchCount count_, const BatchFnRef fn_, const engine::scheduler::Lane lane_) {
	engine::scheduler::parallelFor(*::hg::engine::Scheduler::get(), count_, fn_, lane_);
}
