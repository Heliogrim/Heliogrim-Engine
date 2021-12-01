#include "StaticMeshActor.hpp"

using namespace ember;

StaticMeshActor::StaticMeshActor() = default;

ptr<StaticMeshComponent> StaticMeshActor::getStaticMeshComponent() const noexcept {
    return _staticMeshComponent;
}
