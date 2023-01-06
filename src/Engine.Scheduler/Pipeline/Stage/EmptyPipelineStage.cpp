#include "EmptyPipelineStage.hpp"

using namespace ember::engine::scheduler;
using namespace ember;

EmptyPipelineStage::EmptyPipelineStage(cref<identifier_type> identifier_, pipeline_handle_type pipeline_) :
    PipelineStage(identifier_, pipeline_) {}

EmptyPipelineStage::~EmptyPipelineStage() noexcept = default;

void EmptyPipelineStage::staticDispatch(const non_owning_rptr<const StageDispatcher> dispatcher_) {
    // __noop();
}

void EmptyPipelineStage::dynamicDispatch(const non_owning_rptr<const StageDispatcher> dispatcher_) {
    // __noop();
}
