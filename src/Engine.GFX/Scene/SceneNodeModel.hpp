#pragma once

#include <Engine.Scene/SceneNodeModel.hpp>

namespace ember::engine::gfx {

    class __declspec(novtable) SceneNodeModel :
        public scene::SceneNodeModel {
    public:
        SceneNodeModel(const ptr<SceneComponent> owner_);
    };
}
