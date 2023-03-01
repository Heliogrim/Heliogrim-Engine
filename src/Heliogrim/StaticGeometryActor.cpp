#include "StaticGeometryActor.hpp"

#include "ActorInitializer.hpp"

using namespace hg;

StaticGeometryActor::StaticGeometryActor(cref<ActorInitializer> initializer_) :
    Actor(initializer_),
    _staticGeometryComponent(nullptr) {
    _staticGeometryComponent = initializer_.createComponent<StaticGeometryComponent>(this);
}

ptr<StaticGeometryComponent> StaticGeometryActor::getStaticGeometryComponent() const noexcept {
    return _staticGeometryComponent;
}
