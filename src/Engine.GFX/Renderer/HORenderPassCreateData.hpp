#pragma once
#include <Engine.Scene/IRenderScene.hpp>
#include <Engine.Scheduler/Fiber/Awaitable.hpp>

#include "../Camera/Camera.hpp"
#include "../Texture/Texture.hpp"

namespace ember::engine::gfx::render {

    struct HORenderPassCreateData {

        ptr<Texture> target;

        ptr<scene::IRenderScene> scene = nullptr;

        ptr<Camera> camera = nullptr;
    };
}
