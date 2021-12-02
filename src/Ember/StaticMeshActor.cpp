#include "StaticMeshActor.hpp"

#include "ActorInitializer.hpp"

using namespace ember;

StaticMeshActor::StaticMeshActor() :
    _staticMeshComponent(nullptr) {

    auto& initializer { ActorInitializer::get() };

    _staticMeshComponent = initializer.createComponent<StaticMeshComponent>(this);
};

ptr<StaticMeshComponent> StaticMeshActor::getStaticMeshComponent() const noexcept {
    return _staticMeshComponent;
}
