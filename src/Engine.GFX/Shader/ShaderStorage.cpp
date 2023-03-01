#include "ShaderStorage.hpp"

using namespace hg::engine::gfx;
using namespace hg;

bool ShaderStorageKey::operator==(const ShaderStorageKey& other_) const noexcept {
    return (&other_ == this) || (other_.type == type && other_.name == name);
}

ShaderStorage* ShaderStorage::_instance = nullptr;

void ShaderStorage::store(ptr<Shader> shader_) {

    const ShaderStorageKey key {
        shader_->type(),
        shader_->name()
    };

    _storage.insert_or_assign(key, shader_);
}

bool ShaderStorage::exists(const shader::ShaderType& scope_, const string& name_) const noexcept {

    const ShaderStorageKey key {
        scope_,
        name_
    };

    return _storage.find(key) != _storage.cend();
}

ptr<Shader> ShaderStorage::get(const shader::ShaderType& scope_, const string& name_) const {

    const ShaderStorageKey key {
        scope_,
        name_
    };

    return _storage.at(key);
}

ptr<Shader> ShaderStorage::get(const shader::ShaderType& scope_, const string& name_) {

    const ShaderStorageKey key {
        scope_,
        name_
    };

    return _storage.at(key);
}

ShaderStorage& ShaderStorage::make() noexcept {
    if (!_instance) {
        _instance = new ShaderStorage();
    }
    return *_instance;
}

ref<ShaderStorage> ShaderStorage::get() noexcept {
    return *_instance;
}

void ShaderStorage::destroy() {
    delete _instance;
    _instance = nullptr;
}

void ShaderStorage::tidy() {
    // TODO: Delete stored shaders
    /*
    for (auto& entry : _storage) {
        delete entry.second;
    }
     */
}
