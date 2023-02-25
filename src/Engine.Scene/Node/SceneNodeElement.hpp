#pragma once

#include <concepts>

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Meta/Concept.hpp>

#include "SceneNodePath.hpp"

namespace ember::engine::scene {
    template <class Type_>
    concept IsSceneNodeElement = requires(const Type_ obj_) {
            { obj_.sceneNodePath() } -> decayed_as<SceneNodePath>;
        } && requires(Type_ obj_, const SceneNodePath path_) {
            { obj_.setSceneNodePath(path_) } -> std::same_as<void>;
        };
}
