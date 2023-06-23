#include "AccelerationEffect.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

AccelerationEffect::AccelerationEffect(
    mref<Guid> guid_,
    mref<string> name_,
    mref<Vector<smr<AccelerationStage>>> stages_,
    mref<AccelerationEffectInputs> inputs_,
    mref<AccelerationEffectBindings> bindings_,
    mref<AccelerationEffectOutputs> outputs_
) :
    _guid(_STD move(guid_)),
    _name(_STD move(name_)),
    _stages(_STD move(stages_)),
    _inputs(_STD move(inputs_)),
    _bindings(_STD move(bindings_)),
    _outputs(_STD move(outputs_)) {}

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

cref<AccelerationEffectInputs> AccelerationEffect::getInputLayout() const noexcept {
    return _inputs;
}

cref<AccelerationEffectBindings> AccelerationEffect::getBindingLayout() const noexcept {
    return _bindings;
}

cref<AccelerationEffectOutputs> AccelerationEffect::getOutputLayout() const noexcept {
    return _outputs;
}
