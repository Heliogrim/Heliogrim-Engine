#include "AccelerationSubpassNode.hpp"
#include "../Runtime/AccelerationSubpassNode.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

CompileAccelerationSubpassNode::CompileAccelerationSubpassNode() noexcept = default;

CompileAccelerationSubpassNode::CompileAccelerationSubpassNode(
    mref<uptr<CompileAccelerationComponent>> acceleration_,
    mref<uptr<CompileCommandRecordComponent>> record_
) noexcept :
    CompileSubpassNode() {
    auxAdd(_STD move(acceleration_));
    auxAdd(_STD move(record_));
}

CompileAccelerationSubpassNode::~CompileAccelerationSubpassNode() noexcept = default;

nmpt<const CompileAccelerationComponent> CompileAccelerationSubpassNode::getAcceleration() const noexcept {
    return _auxiliary->getFirstOfType<CompileAccelerationComponent>();
}

nmpt<const CompileCommandRecordComponent> CompileAccelerationSubpassNode::getCommandRecord() const noexcept {
    return _auxiliary->getFirstOfType<CompileCommandRecordComponent>();
}

uptr<RuntimeNode> CompileAccelerationSubpassNode::compile(cref<CompilePassContext> ctx_) const noexcept {

    const auto compileAcceleration = getAcceleration();
    auto runtimeAcceleration = compileAcceleration->compile(ctx_);

    const auto compileRecord = getCommandRecord();
    auto runtimeRecord = compileRecord->compile(ctx_);

    const auto& allocator = ctx_.getGraphNodeAllocator();
    return allocator->allocate<AccelerationSubpassNode>(
        _STD move(runtimeAcceleration),
        _STD move(runtimeRecord)
    );
}
