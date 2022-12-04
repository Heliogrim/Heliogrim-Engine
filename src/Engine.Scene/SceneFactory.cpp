#include "SceneFactory.hpp"

#include <Engine.Common/Wrapper.hpp>

#include "RevScene.hpp"

using namespace ember::engine::scene;
using namespace ember;

sptr<Scene> SceneFactory::createDefaultScene() {
    return make_sptr<RevScene>();
}

sptr<Scene> SceneFactory::createPreviewScene() {
    return nullptr;
}
