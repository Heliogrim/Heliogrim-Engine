#pragma once
#include <Engine.Common/Math/Bounding.hpp>
#include <Engine.Common/Math/Transformation.hpp>
#include <Engine.Proxy/ProxiedScenePayload.hpp>

namespace ember::engine::scene {

    struct SceneNodeCreateData {
        proxy::ProxiedScenePayload payload;

        math::Transformation transformation;

        math::Bounding bounding;
    };

}
