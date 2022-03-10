#pragma once

#include <Engine.Scene/SceneNodeModel.hpp>

#include "../ModelBatch.hpp"
#include "../GraphicPass/GraphicPassMask.hpp"

namespace ember::engine::gfx {

    class __declspec(novtable) SceneNodeModel :
        public scene::SceneNodeModel {
    public:
        SceneNodeModel(const ptr<SceneComponent> owner_);

    public:
        [[nodiscard]] virtual ModelBatch batch(const GraphicPassMask mask_) = 0;
    };
}
