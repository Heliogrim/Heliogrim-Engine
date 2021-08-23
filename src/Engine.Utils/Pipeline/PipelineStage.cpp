#include "PipelineStage.hpp"
#include "./Pipeline.hpp"

using namespace ember::engine::utils;

PipelineStage::PipelineStage(Pipeline& pipeline_, Dispatchable& dispatchable_) :
    _pipeline(&pipeline_),
    _dispatchable(&dispatchable_) { }

bool PipelineStage::running() const {
    // TODO: return _running.operator bool();
    return _running;
}

void PipelineStage::invoke() {
    // TODO: _running.exchange(true);
    _running = true;
    _dispatchable->before();

    const uint32_t batches = _dispatchable->batches();
    for (uint32_t i = 0; i < batches; ++i) {
        /**
         * Create a task for each batches index
         */
        _dispatchable->invoke(i);

        /**
         * Push the task to tasking pool
         */
        // TODO: tasking->addTask(tasks[i]);
    }

    // Temporary
    _dispatchable->after();
    // TODO: _running.exchange(false);
    _running = false;
}
