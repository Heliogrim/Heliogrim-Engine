#include "EmptyPipelineStage.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

EmptyPipelineStage::EmptyPipelineStage(cref<identifier_type> identifier_, pipeline_handle_type pipeline_) :
    PipelineStage(identifier_, pipeline_) {}

EmptyPipelineStage::~EmptyPipelineStage() noexcept = default;

void EmptyPipelineStage::staticDispatch(cref<StageDispatcher> dispatcher_) {
    // __noop();
}

void EmptyPipelineStage::dynamicDispatch(cref<StageDispatcher> dispatcher_) {
    // __noop();
}
