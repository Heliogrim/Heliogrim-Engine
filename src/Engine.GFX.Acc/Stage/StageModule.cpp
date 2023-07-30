#include "StageModule.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

StageModule::StageModule(mref<uptr<NativeModule>> native_) noexcept :
    _native(_STD move(native_)) {}

StageModule::~StageModule() = default;

non_owning_rptr<const StageModule::NativeModule> StageModule::getNativeModule() const noexcept {
    return _native.get();
}
