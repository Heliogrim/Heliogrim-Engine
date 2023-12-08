#include "Material.hpp"

#include <cassert>

#include "MaterialPrototype.hpp"

using namespace hg::engine::gfx::material;
using namespace hg;

Material::Material() noexcept :
    _guid(),
    _prototype(),
    _parameters() {}

Material::Material(
    mref<Guid> guid_,
    mref<smr<MaterialPrototype>> prototype_,
    mref<Vector<MaterialParameter>> parameters_
) noexcept :
    _guid(_STD move(guid_)),
    _prototype(_STD move(prototype_)),
    _parameters(_STD move(parameters_)) {}

Material::~Material() = default;

Guid Material::getGuid() const noexcept {
    return _guid;
}

smr<MaterialPrototype> Material::getPrototype() const noexcept {
    return _prototype;
}

cref<Vector<MaterialParameter>> Material::getParameters() const noexcept {
    return _parameters;
}

bool Material::hasParam(string_view name_) const noexcept {

    assert(_prototype);
    const auto& mpp = _prototype->getParameters();
    const auto mppit = _STD ranges::find(
        mpp,
        name_,
        [](cref<MaterialPrototypeParameter> param_) {
            return param_.getName();
        }
    );

    return mppit != mpp.end();
}

Material Material::clone() const noexcept {
    return Material {};
}
