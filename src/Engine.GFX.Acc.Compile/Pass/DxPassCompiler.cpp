#include "DxPassCompiler.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

DxPassCompiler::DxPassCompiler() = default;

DxPassCompiler::~DxPassCompiler() = default;

smr<const AccelerationPass> DxPassCompiler::compile(
    cref<class SpecificationStorage> specifications_,
    mref<smr<AccelerationPass>> source_,
    mref<Vector<smr<AccelerationStageDerivat>>> stages_,
    mref<Vector<uptr<CompiledModule>>> modules_
) const {
    return nullptr;
}
