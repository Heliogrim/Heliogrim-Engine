#include "PipelineStage.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

PipelineStage::PipelineStage(cref<identifier_type> identifier_, pipeline_handle_type pipeline_) noexcept :
    _stageIdentifier(identifier_),
    _stagePipeline(pipeline_) {}

PipelineStage::~PipelineStage() noexcept = default;

cref<PipelineStage::identifier_type> PipelineStage::stageIdentifier() const noexcept {
    return _stageIdentifier;
}

PipelineStage::pipeline_handle_type PipelineStage::stagePipeline() const noexcept {
    return _stagePipeline;
}
