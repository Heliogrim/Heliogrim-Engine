#include "SceneProxiedRef.hpp"

#include <Engine.Common/Make.hpp>

using namespace ember::engine::proxy;
using namespace ember;

SceneProxiedRef::SceneProxiedRef(non_owning_rptr<ActorComponent> owner_, mref<uptr<SceneProxy>> proxy_,
    mref<OwningProxiedScenePayload> payload_) noexcept :
    _owner(owner_),
    _proxy(_STD move(proxy_)),
    _payload(_STD move(payload_)) {}

SceneProxiedRef::~SceneProxiedRef() noexcept = default;

cref<OwningProxiedScenePayload> SceneProxiedRef::payload() const noexcept {
    return _payload;
}

ref<OwningProxiedScenePayload> SceneProxiedRef::payload() noexcept {
    return _payload;
}

managed<SceneProxiedRef> SceneProxiedRef::make_proxied_ref(
    mref<uptr<SceneProxy>> proxy_, non_owning_rptr<ActorComponent> owner_
) {
    auto* tmp { new SceneProxiedRef(owner_, _STD move(proxy_), OwningProxiedScenePayload { nullptr }) };
    return sptr<SceneProxiedRef> { tmp };
}
