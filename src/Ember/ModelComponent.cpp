#include "ModelComponent.hpp"

using namespace ember;

ModelComponent::ModelComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_) :
    SceneComponent(_STD move(owner_), _STD move(parent_)) {}
