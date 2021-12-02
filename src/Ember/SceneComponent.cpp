#include "SceneComponent.hpp"

using namespace ember;

SceneComponent::SceneComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_) :
    ActorComponent(_STD move(owner_), _STD move(parent_)) {}
