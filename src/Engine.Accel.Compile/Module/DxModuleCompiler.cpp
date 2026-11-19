#include "DxModuleCompiler.hpp"

import Engine.Asserts;

using namespace hg::engine::accel;
using namespace hg;

DxModuleCompiler::~DxModuleCompiler() noexcept = default;

uptr<CompiledModule> DxModuleCompiler::compile(
	cref<smr<AccelerationPipeline>> targetPass_,
	cref<smr<const EffectSpecification>> specifications_,
	mref<uptr<ModuleSource>> source_
) const {
	::hg::todo_panic();
}
