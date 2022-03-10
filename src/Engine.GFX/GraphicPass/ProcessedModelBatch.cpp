#include "ProcessedModelBatch.hpp"

using namespace ember::engine::gfx;
using namespace ember;

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

cref<Vector<ExecutionRow>> ProcessedModelBatch::executions() const noexcept {
    return _executions;
}

ref<Vector<ExecutionRow>> ProcessedModelBatch::executions() noexcept {
    return _executions;
}
