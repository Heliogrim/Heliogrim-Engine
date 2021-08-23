#include "SceneElement.hpp"

#include <Engine.Proxy/SceneProxiedRef.hpp>

using namespace ember::engine::scene;
using namespace ember;

wptr<engine::proxy::SceneProxiedRef> SceneElement::owner() const noexcept {
    return _owner;
}

cref<engine::gfx::ModelBatch> SceneElement::batch(cref<gfx::GraphicPassMask> mask_) const {
    return _batches[static_cast<u8>(mask_)];
}

ref<engine::gfx::ModelBatch> SceneElement::batch(cref<gfx::GraphicPassMask> mask_) {
    return _batches[static_cast<u8>(mask_)];
}

ptr<const engine::gfx::IModelState> SceneElement::state() const noexcept {
    return _state;
}

ptr<engine::gfx::IModelState> SceneElement::state() noexcept {
    return _state;
}
