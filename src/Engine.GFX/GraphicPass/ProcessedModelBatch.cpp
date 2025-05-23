#include "ProcessedModelBatch.hpp"

using namespace hg::engine::gfx;
using namespace hg;

bool ProcessedModelBatch::empty() const noexcept {
    return _executions.empty();
}

cref<SharedGeometry> ProcessedModelBatch::geometry() const noexcept {
    return _geometry;
}

ref<SharedGeometry> ProcessedModelBatch::geometry() noexcept {
    return _geometry;
}

cref<SharedBind> ProcessedModelBatch::bind() const noexcept {
    return _bind;
}

ref<SharedBind> ProcessedModelBatch::bind() noexcept {
    return _bind;
}

cref<Vector<ExecutionRow>> ProcessedModelBatch::executions() const noexcept {
    return _executions;
}

ref<Vector<ExecutionRow>> ProcessedModelBatch::executions() noexcept {
    return _executions;
}
