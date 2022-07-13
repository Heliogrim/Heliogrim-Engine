#include "SceneComponent.hpp"

using namespace ember;

SceneComponent::SceneComponent(mref<component_type_id> typeId_, mref<CachedActorPointer> owner_,
    mref<ptr<ActorComponent>> parent_) :
    ActorComponent(_STD move(typeId_), _STD move(owner_), _STD move(parent_)) {}

SceneComponent::~SceneComponent() {
    for (auto&& model : _sceneNodeModels) {
        #ifdef _DEBUG
        assert(!model->markAsDeleted());
        #else
        [[maybe_unused]] auto marked { model->markAsDeleted() };
        #endif
    }
}

cref<CompactSet<non_owning_rptr<SceneNodeModel>>> SceneComponent::getSceneNodeModels() const noexcept {
    return _sceneNodeModels;
}

void SceneComponent::addSceneNodeModel(const non_owning_rptr<SceneNodeModel> model_) {
    _sceneNodeModels.insert(model_);
}
