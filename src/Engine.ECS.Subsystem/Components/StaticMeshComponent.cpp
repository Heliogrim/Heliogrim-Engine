#include "StaticMeshComponent.hpp"

#include <Engine.Common/stdafx.h>
#include <Engine.GFX/Scene/SceneElement.hpp>
#include <Engine.Scene/Node/SceneNode.hpp>

#include "Engine.Proxy/SceneProxiedRef.hpp"
#include "Engine.Scene/Scene.hpp"
#include "Engine.Session/Session.hpp"

using namespace ember::engine::ecs::subsystem;
using namespace ember;

void StaticMeshComponent::mantle(cref<entity_guid> entity_) {
    DEBUG_MSG("Mantle entity with StaticMeshComponent.")

    gfx::SceneElement se {};
    scene::SceneNodeElementBase sneb;
    scene::SceneNode sn {};

    auto* prx = new proxy::StaticModelSceneProxy();
    auto spr = proxy::SceneProxiedRef::make_proxied_ref(prx, this);

    sn.element() = sneb;

    ptr<scene::Scene> scene = Session::get()->scene();

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
}

void StaticMeshComponent::dismantle(cref<entity_guid> entity_) {
    DEBUG_MSG("Dismantle entity with StaticMeshComponent.")
}

cref<asset_guid> StaticMeshComponent::mesh() const noexcept {
    return _meshGuid;
}

cref<asset_guid> StaticMeshComponent::material() const noexcept {
    return _materialGuid;
}
