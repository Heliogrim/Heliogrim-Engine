#include "MaterialResource.hpp"

#include <Engine.GFX/Buffer/VirtualBufferView.hpp>

using namespace hg::engine::gfx::loader;
using namespace hg;

Material::Material() noexcept = default;

Material::~Material() = default;

cref<smr<engine::gfx::TextureResource>> Material::diffuse() const noexcept {
    return _diffuse;
}

cref<smr<engine::gfx::TextureResource>> Material::normal() const noexcept {
    return _normal;
}

cref<smr<engine::gfx::TextureResource>> Material::roughness() const noexcept {
    return _roughness;
}

cref<smr<engine::gfx::TextureResource>> Material::metalness() const noexcept {
    return _metalness;
}

cref<smr<engine::gfx::TextureResource>> Material::ao() const noexcept {
    return _ao;
}

cref<smr<engine::gfx::TextureResource>> Material::alpha() const noexcept {
    return _alpha;
}

const non_owning_rptr<const engine::gfx::VirtualBufferView> Material::view() const noexcept {
    return _view.get();
}
