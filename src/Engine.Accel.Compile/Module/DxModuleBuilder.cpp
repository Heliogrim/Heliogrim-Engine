#include "DxModuleBuilder.hpp"

import Engine.Asserts;

using namespace hg::engine::accel;
using namespace hg;

DxModuleBuilder::~DxModuleBuilder() noexcept = default;

uptr<ModuleSource> DxModuleBuilder::build(
	cref<smr<AccelerationPipeline>> targetPass_,
	cref<smr<const EffectSpecification>> specifications_,
	cref<smr<StageDerivat>> stage_,
	cref<uptr<ModuleSource>> previous_
) const {
	::hg::todo_panic();
}
