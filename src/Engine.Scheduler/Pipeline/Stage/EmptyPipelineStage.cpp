#include "EmptyPipelineStage.hpp"

#include <Engine.Common/Move.hpp>

using namespace hg::engine::scheduler;
using namespace hg;

EmptyPipelineStage::EmptyPipelineStage(
	cref<identifier_type> identifier_,
	pipeline_handle_type pipeline_,
	mref<graph::StageDecl> decl_
) :
	PipelineStage(identifier_, pipeline_),
	_decl(::hg::move(decl_)) {}

EmptyPipelineStage::~EmptyPipelineStage() noexcept = default;

void EmptyPipelineStage::staticDispatch([[maybe_unused]] cref<StaticStageDispatcher> dispatcher_) {
	// __noop();
}

void EmptyPipelineStage::dynamicDispatch([[maybe_unused]] cref<DynamicStageDispatcher> dispatcher_) {
	// __noop();
}

graph::StageDecl EmptyPipelineStage::decl() const noexcept {
	return _decl;
}
