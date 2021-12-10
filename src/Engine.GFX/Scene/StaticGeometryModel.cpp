#include "StaticGeometryModel.hpp"

#include <Ember/StaticGeometryComponent.hpp>
#include <Engine.Assets/Database/AssetDatabaseQuery.hpp>
#include <Engine.Scene/RevScene.hpp>

#include "Engine.Resource/ResourceManager.hpp"

using namespace ember::engine::gfx;
using namespace ember;

StaticGeometryModel::StaticGeometryModel(const ptr<SceneComponent> owner_) :
    SceneNodeModel(owner_) {}

void StaticGeometryModel::create(const ptr<scene::Scene> scene_) {

    /**
     *
     */
    auto* const scene { static_cast<const ptr<scene::RevScene>>(scene_) };
    auto& graph { scene->renderGraph()->asMutable() };

    auto* origin { static_cast<ptr<StaticGeometryComponent>>(_owner) };

    /**
     *
     */
    _boundary = origin->getBoundaries();
    _staticGeometry = static_cast<ptr<assets::StaticGeometry>>(origin->getStaticGeometryAsset().internal());
    _staticGeometryResource = Session::get()->modules().resourceManager()->loader().load(_staticGeometry, nullptr);

    /**
     *
     */
    graph.push(this);
}

void StaticGeometryModel::update(const ptr<scene::Scene> scene_) {}

void StaticGeometryModel::destroy(const ptr<scene::Scene> scene_) {}
