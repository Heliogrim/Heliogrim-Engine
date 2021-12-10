#include "StaticGeometryActor.hpp"

#include "ActorInitializer.hpp"

using namespace ember;

StaticGeometryActor::StaticGeometryActor() :
    _staticGeometryComponent(nullptr) {

    auto& initializer { ActorInitializer::get() };

    _staticGeometryComponent = initializer.createComponent<StaticGeometryComponent>(this);
};

ptr<StaticGeometryComponent> StaticGeometryActor::getStaticGeometryComponent() const noexcept {
    return _staticGeometryComponent;
}
