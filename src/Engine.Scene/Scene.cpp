#include "Scene.hpp"

#include <Engine.GFX/Scene/SceneTag.hpp>

using namespace ember::engine::scene;
using namespace ember;

Scene::Scene() noexcept = default;

Scene::~Scene() = default;

void Scene::update() {}

void Scene::setNodeType(const SceneTag tag_, cref<component_type_id> typeId_,
    cref<NodeTypeRegistryValue::model_type> model_) {

    if (constexpr gfx::GfxSceneTag gfxSceneTag {}; tag_ != gfxSceneTag) {
        return;
    }

    /**
     *
     */
    _nodeTypeRegistry.setGfxModel(typeId_, model_);
}

ptr<World> Scene::getWorld() noexcept {
    return nullptr;
}
