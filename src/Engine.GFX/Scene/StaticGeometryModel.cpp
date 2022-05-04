#include "StaticGeometryModel.hpp"

#include <Ember/StaticGeometryComponent.hpp>
#include <Engine.Assets/Database/AssetDatabaseQuery.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scene/RevScene.hpp>

#include "../Resource/StaticGeometryResource.hpp"

using namespace ember::engine::gfx;
using namespace ember;

StaticGeometryModel::StaticGeometryModel(const ptr<SceneComponent> owner_) :
    SceneNodeModel(owner_),
    __tmp__instance(),
    __tmp__cdb(nullptr) {}

StaticGeometryModel::~StaticGeometryModel() {
    tidy();
}

void StaticGeometryModel::tidy() {

    if (__tmp__instance.buffer) {
        __tmp__instance.destroy();
    }

    if (__tmp__cdb) {
        delete __tmp__cdb;
        __tmp__cdb = nullptr;
    }
}

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
    _staticGeometryAsset = static_cast<ptr<assets::StaticGeometry>>(origin->getStaticGeometryAsset().internal());
    _staticGeometryResource = Session::get()->modules().resourceManager()->loader().load(_staticGeometryAsset, nullptr);

    /**
     *
     */
    graph.push(this);
}

void StaticGeometryModel::update(const ptr<scene::Scene> scene_) {}

void StaticGeometryModel::destroy(const ptr<scene::Scene> scene_) {}

const ptr<engine::assets::StaticGeometry> StaticGeometryModel::geometryAsset() const noexcept {
    return _staticGeometryAsset;
}

const ptr<engine::res::Resource> StaticGeometryModel::geometryResource() const noexcept {
    return _staticGeometryResource;
}

bool StaticGeometryModel::streamable() const noexcept {
    return _streamable;
}

ModelBatch StaticGeometryModel::batch(const GraphicPassMask mask_) {

    // TODO: Change getting transform information to resolve via render graph
    auto* origin { static_cast<ptr<StaticGeometryComponent>>(_owner) };
    auto* res { static_cast<ptr<StaticGeometryResource>>(_staticGeometryResource) };

    return ModelBatch {
        origin->getWorldTransform(),
        res->_vertexData.buffer,
        res->_indexData.buffer,
        static_cast<u32>(res->_indexData.buffer.size / sizeof(u32)),
        0ui32
    };
}
