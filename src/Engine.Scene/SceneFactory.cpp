#include "SceneFactory.hpp"

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Make.hpp>

#include "RevScene.hpp"

using namespace ember::engine::scene;
using namespace ember;

uptr<Scene> SceneFactory::createDefaultScene() {
    return make_uptr<RevScene>();
}

uptr<Scene> SceneFactory::createPreviewScene() {
    return nullptr;
}
