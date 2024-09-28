#include "EmptyPipelineStage.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

EmptyPipelineStage::EmptyPipelineStage(cref<identifier_type> identifier_, pipeline_handle_type pipeline_) :
	PipelineStage(identifier_, pipeline_) {}

EmptyPipelineStage::~EmptyPipelineStage() noexcept = default;

void EmptyPipelineStage::staticDispatch([[maybe_unused]] cref<StaticStageDispatcher> dispatcher_) {
	// __noop();
}

void EmptyPipelineStage::dynamicDispatch([[maybe_unused]] cref<DynamicStageDispatcher> dispatcher_) {
	// __noop();
}
