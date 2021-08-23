#include "Prototype.hpp"

using namespace ember::engine::gfx::shader;
using namespace ember;

Prototype::Prototype() noexcept = default;

Prototype::Prototype(const ShaderType& type_, const string& name_) noexcept :
    _type(type_),
    _name(name_),
    _code(nullptr),
    _codeSize(0) { }

Prototype::~Prototype() noexcept = default;

ShaderType Prototype::type() const noexcept {
    return _type;
}

const string& Prototype::name() const noexcept {
    return _name;
}

const std::vector<PrototypeBinding>& Prototype::bindings() const noexcept {
    return _bindings;
}

void Prototype::add(const PrototypeBinding& binding_) {
    _bindings.push_back(binding_);
}

u64 Prototype::codeSize() const noexcept {
    return _codeSize;
}

ptr<void> Prototype::code() const noexcept {
    return _code;
}

void Prototype::storeCode(ptr<void> data_, const u64 size_) noexcept {
    _code = data_;
    _codeSize = size_;
}
