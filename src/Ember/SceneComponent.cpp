#include "SceneComponent.hpp"

using namespace ember;

SceneComponent::SceneComponent(mref<CachedActorPointer> owner_, const ptr<ActorComponent> parent_) :
    ActorComponent(_STD move(owner_), parent_) {}
