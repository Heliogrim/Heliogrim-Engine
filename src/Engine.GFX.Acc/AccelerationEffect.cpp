#include "AccelerationEffect.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

AccelerationEffect::AccelerationEffect(
    mref<Guid> guid_,
    mref<string> name_,
    mref<Vector<smr<AccelerationStage>>> stages_,
    mref<AccelerationInputLayout> inputLayout_,
    mref<AccelerationBindingLayout> bindingLayout_,
    mref<AccelerationOutputLayout> outputLayout_
) :
    _guid(_STD move(guid_)),
    _name(_STD move(name_)),
    _stages(_STD move(stages_)),
    _inputLayout(_STD move(inputLayout_)),
    _bindingLayout(_STD move(bindingLayout_)),
    _outputLayout(_STD move(outputLayout_)) {}

AccelerationEffect::~AccelerationEffect() = default;

Guid AccelerationEffect::getGuid() const noexcept {
    return _guid;
}

string AccelerationEffect::getName() const noexcept {
    return _name;
}

cref<Vector<smr<AccelerationStage>>> AccelerationEffect::getStages() const noexcept {
    return _stages;
}

cref<AccelerationInputLayout> AccelerationEffect::getInputLayout() const noexcept {
    return _inputLayout;
}

cref<AccelerationBindingLayout> AccelerationEffect::getBindingLayout() const noexcept {
    return _bindingLayout;
}

cref<AccelerationOutputLayout> AccelerationEffect::getOutputLayout() const noexcept {
    return _outputLayout;
}
