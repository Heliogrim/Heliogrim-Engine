#include "AccelerationStageModule.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

AccelerationStageModule::AccelerationStageModule(mref<uptr<NativeModule>> native_) noexcept :
    _native(_STD move(native_)) {}

AccelerationStageModule::~AccelerationStageModule() = default;

non_owning_rptr<const AccelerationStageModule::NativeModule> AccelerationStageModule::getNativeModule() const noexcept {
    return _native.get();
}
