#include "StaticMeshComponent.hpp"

#include <Engine.Common/stdafx.h>
#include <Engine.ECS/Pool.hpp>
#include <Engine.GFX/Scene/SceneElement.hpp>
#include <Engine.GFX/Scene/SceneGraphTag.hpp>
#include <Engine.Proxy/StaticModelSceneProxy.hpp>
#include <Engine.Scene/Scene.hpp>
#include <Engine.Scene/Graph/MutableSceneGraph.hpp>
#include <Engine.Scene/Node/SceneNode.hpp>
#include <Engine.Session/Session.hpp>

#include "TransformComponent.hpp"

using namespace ember::engine::ecs::subsystem;
using namespace ember;

void StaticMeshComponent::mantle(cref<entity_guid> entity_) {
    DEBUG_MSG("Mantle entity with StaticMeshComponent.")

    auto prx = make_uptr<proxy::StaticModelSceneProxy>();
    auto spr = proxy::SceneProxiedRef::make_proxied_ref(_STD move(prx), this);

    ptr<scene::Scene> scene = Session::get()->scene();
    auto* graph = scene->getGraph<gfx::GfxSceneGraphTag>();
    auto mgraph { graph->asMutable() };

    auto sceneElement = make_sptr<engine::gfx::SceneElement>();
    auto payload = engine::proxy::OwningProxiedScenePayload(sceneElement);

    auto tc = pool<TransformComponent>::get().get(entity_);
    if (tc == nullptr) {
        DEBUG_SNMSG(false, "LOG",
            "Instantiating StaticMeshProxy without related TransformComponent. Fallback to ZeroTransformation...")
    }

    [[maybe_unused]] const auto* node = mgraph.push(engine::scene::SceneNodeCreateData {
        .payload = payload,
        .transformation = tc != nullptr ? tc->transformation() : math::ZeroTransformation {},
        .bounding = {}
    });

    spr->payload() = _STD move(payload);

    /**
     * 1. create a static model scene proxy
     * 2. create scene proxied reference to link ecs, scene graph and payload
     * 3. create scene node
     * 4. link scene element against scene node
     * 5. push scene node to scene graph
     */

    /**
     * Enitity -> [Components]
     * Component -> [Data | Proxy]
     * Proxy ~> [{Component} | SceneProxy | {SceneNodeElement}]
     */
    _proxy = _STD move(spr);
}

void StaticMeshComponent::dismantle([[maybe_unused]] cref<entity_guid> entity_) {
    DEBUG_MSG("Dismantle entity with StaticMeshComponent.")

    if (_proxy.use_count() > 0) {
        _proxy.reset();
    }
}

cref<asset_guid> StaticMeshComponent::mesh() const noexcept {
    return _meshGuid;
}

cref<asset_guid> StaticMeshComponent::material() const noexcept {
    return _materialGuid;
}
