#include "MaterialParameter.hpp"

using namespace hg::engine::gfx::material;
using namespace hg;

MaterialParameter::MaterialParameter(mref<uptr<MaterialParameterStorageBase>> storage_) noexcept :
    _storage(_STD move(storage_)) {}

MaterialParameter::~MaterialParameter() = default;

nmpt<const HeliogrimClass> MaterialParameter::getStorageClass() const noexcept {
    if (_storage) {
        return { _storage->getClass() };
    }
    return {};
}
