#include "SceneFactory.hpp"

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Make.hpp>

#include "RevScene.hpp"

using namespace hg::engine::scene;
using namespace hg;

uptr<Scene> SceneFactory::createDefaultScene() {
    return uptr<Scene>(HeliogrimObject::create<RevScene>());
}

uptr<Scene> SceneFactory::createPreviewScene() {
    return nullptr;
}
