#include "SceneProxiedRef.hpp"

#include <Engine.Common/Make.hpp>

using namespace ember::engine::proxy;
using namespace ember;

managed<SceneProxiedRef> SceneProxiedRef::make_proxied_ref(
    ptr<SceneProxy> proxy_, non_owning_rptr<ecs::subsystem::SceneComponent> owner_
) {
    auto man { make_sptr<SceneProxiedRef>(owner_, proxy_, nullptr) };
    return man;
}
