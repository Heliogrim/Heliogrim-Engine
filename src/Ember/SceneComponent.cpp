#include "SceneComponent.hpp"

using namespace ember;

SceneComponent::SceneComponent(mref<component_type_id> typeId_, mref<CachedActorPointer> owner_,
    mref<ptr<ActorComponent>> parent_) :
    ActorComponent(_STD move(typeId_), _STD move(owner_), _STD move(parent_)) {}

cref<CompactSet<ptr<SceneNodeModel>>> SceneComponent::getSceneNodeModels() const noexcept {
    return _sceneNodeModels;
}

void SceneComponent::addSceneNodeModel(const ptr<SceneNodeModel> model_) {
    _sceneNodeModels.insert(model_);
}
