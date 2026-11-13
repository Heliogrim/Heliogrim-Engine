#include "StagePipeline.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

StagePipeline::StagePipeline(mref<identifier_type> identifier_) noexcept :
	_identifier(std::move(identifier_)) {}

StagePipeline::~StagePipeline() noexcept = default;

cref<StagePipeline::identifier_type> StagePipeline::identifier() const noexcept {
	return _identifier;
}

bool StagePipeline::isSharedTask(const non_owning_rptr<const task_type> task_) const noexcept {
	// TODO:
	return false;
}

bool StagePipeline::isExclusiveTask(const non_owning_rptr<const task_type> task_) const noexcept {
	// TODO:
	return false;
}
