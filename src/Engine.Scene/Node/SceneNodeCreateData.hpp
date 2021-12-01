#pragma once
#include <Engine.Common/Math/Bounding.hpp>
#include <Engine.Common/Math/Transform.hpp>
#include <Engine.Proxy/ProxiedScenePayload.hpp>

namespace ember::engine::scene {

    struct SceneNodeCreateData {
        proxy::ProxiedScenePayload payload;

        math::Transform transform;

        math::Bounding bounding;
    };

}
