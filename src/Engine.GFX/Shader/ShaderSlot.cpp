#include "ShaderSlot.hpp"

#include "ShaderStorage.hpp"

using namespace hg::engine::gfx;
using namespace hg;

ShaderSlot::ShaderSlot(const shader::ShaderType type_, const string& name_) noexcept :
    _cache(nullptr),
    _type(type_),
    _name(name_) {}

ptr<const Shader> ShaderSlot::shader(bool bypass_) const noexcept {
    if (_cache && !bypass_) {
        return _cache;
    }

    auto& storage = ShaderStorage::get();

    _cache = nullptr;
    if (storage.exists(_type, _name)) {
        _cache = storage.get(_type, _name);
    }

    return _cache;
}

ptr<const Shader> ShaderSlot::shader(bool bypass_) noexcept {
    if (_cache && !bypass_) {
        return _cache;
    }

    auto& storage = ShaderStorage::get();

    _cache = nullptr;
    if (storage.exists(_type, _name)) {
        _cache = storage.get(_type, _name);
    }

    return _cache;
}

shader::ShaderType ShaderSlot::type() const noexcept {
    return _type;
}

const string& ShaderSlot::name() const noexcept {
    return _name;
}

string& ShaderSlot::name() noexcept {
    return _name;
}
