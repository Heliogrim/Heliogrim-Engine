#include "SceneElement.hpp"

#include <Engine.Proxy/SceneProxiedRef.hpp>

using namespace ember::engine::gfx;
using namespace ember;

wptr<engine::proxy::SceneProxiedRef> SceneElement::owner() const noexcept {
    return _owner;
}

cref<ModelBatch> SceneElement::batch(cref<GraphicPassMask> mask_) const {
    return _batches[static_cast<u8>(mask_)];
}

ref<ModelBatch> SceneElement::batch(cref<GraphicPassMask> mask_) {
    return _batches[static_cast<u8>(mask_)];
}

ptr<const IModelState> SceneElement::state() const noexcept {
    return _state;
}

ptr<IModelState> SceneElement::state() noexcept {
    return _state;
}
