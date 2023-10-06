#include "MaterialPrototype.hpp"

#include <algorithm>

#include "Material.hpp"
#include "MaterialPrototypeParameter.hpp"

using namespace hg::engine::gfx::material;
using namespace hg::engine::gfx::acc;
using namespace hg;

MaterialPrototype::MaterialPrototype(
    mref<Guid> guid_,
    mref<string> name_,
    mref<Vector<smr<AccelerationEffect>>> effects_,
    mref<Vector<MaterialPrototypeParameter>> parameters_
) noexcept :
    _guid(_STD move(guid_)),
    _name(_STD move(name_)),
    _effects(_STD move(effects_)),
    _parameters(_STD move(parameters_)) {}

MaterialPrototype::~MaterialPrototype() = default;

Guid MaterialPrototype::getGuid() const noexcept {
    return _guid;
}

string MaterialPrototype::getName() const noexcept {
    return _name;
}

cref<Vector<smr<AccelerationEffect>>> MaterialPrototype::getAccelerationEffects() const noexcept {
    return _effects;
}

cref<Vector<MaterialPrototypeParameter>> MaterialPrototype::getParameters() const noexcept {
    return _parameters;
}

bool MaterialPrototype::addParameter(mref<MaterialPrototypeParameter> param_) {

    const auto iter = _STD ranges::find(
        _parameters,
        param_.getUniqueName(),
        [](cref<MaterialPrototypeParameter> entry_) {
            return entry_.getUniqueName();
        }
    );

    if (iter != _parameters.end()) {
        return false;
    }

    _parameters.push_back(_STD move(param_));
    return true;
}

bool MaterialPrototype::removeParameter(string_view uniqueName_) {

    const auto range = _STD ranges::remove(
        _parameters,
        uniqueName_,
        [](cref<MaterialPrototypeParameter> entry_) {
            return entry_.getUniqueName();
        }
    );

    _parameters.erase(range.begin(), range.end());
    return range.begin() != _parameters.end();
}

smr<Material> MaterialPrototype::instantiate() const noexcept {
    return smr<Material> {};
}
