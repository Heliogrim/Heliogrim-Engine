#include "SceneNodeFactory.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>

using namespace ember::engine::scene;
using namespace ember;

SceneNodeFactory::SceneNodeFactory(sptr<EmberSceneNodeStorage> storage_) noexcept :
    _storage(storage_) {}

sptr<EmberSceneNodeStorage> SceneNodeFactory::storage() const noexcept {
    return _storage;
}

SceneNodeFactory::factory_assemble_result SceneNodeFactory::assembleRoot() const {

    auto nodeId = _nodeIdGen.fetch_add(1);
    auto stored = _storage->insert(nodeId, {});

    if (!stored.second) {
        throw _STD bad_alloc();
    }

    // TODO: Replace, this is only temporary
    SceneNodeHead head { nodeId };
    head.storage = _storage.get();

    return {
        _STD move(head),
        stored.first
    };
}

SceneNodeFactory::factory_assemble_result SceneNodeFactory::assembleShadow() const {
    throw NotImplementedException();
}

SceneNodeFactory::factory_assemble_result SceneNodeFactory::assembleLoosy() const {
    throw NotImplementedException();
}

SceneNodeFactory::factory_assemble_result SceneNodeFactory::assembleSpartial() const {
    throw NotImplementedException();
}

SceneNodeFactory::factory_assemble_result SceneNodeFactory::assembleNatural() const {
    throw NotImplementedException();
}
