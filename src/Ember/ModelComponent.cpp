#include "ModelComponent.hpp"

using namespace ember;

ModelComponent::ModelComponent(mref<CachedActorPointer> owner_, const ptr<ActorComponent> parent_) :
    SceneComponent(_STD move(owner_), parent_) {}
