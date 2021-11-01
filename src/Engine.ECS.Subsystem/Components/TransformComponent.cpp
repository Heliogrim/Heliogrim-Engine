#include "TransformComponent.hpp"

using namespace ember::engine::ecs::subsystem;
using namespace ember;

cref<math::Transformation> TransformComponent::transformation() const noexcept {
    return _transform;
}
