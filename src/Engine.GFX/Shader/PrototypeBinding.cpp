#include "PrototypeBinding.hpp"

using namespace hg::engine::gfx::shader;
using namespace hg::engine::gfx;
using namespace hg;

PrototypeBinding::PrototypeBinding(
    const BindingType& type_,
    const u32 id_,
    const BindingUpdateInterval interval_,
    const string& name_
) noexcept :
    _type(type_),
    _id(id_),
    _interval(interval_),
    _name(name_) {}

PrototypeBinding::~PrototypeBinding() noexcept = default;

BindingType PrototypeBinding::type() const noexcept {
    return _type;
}

u32 PrototypeBinding::id() const noexcept {
    return _id;
}

BindingUpdateInterval PrototypeBinding::updateInterval() const noexcept {
    return _interval;
}

const string& PrototypeBinding::name() const noexcept {
    return _name;
}

ShaderType PrototypeBinding::shaderType() const noexcept {
    return _shaderType;
}

ShaderType& PrototypeBinding::shaderType() noexcept {
    return _shaderType;
}
