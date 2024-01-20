#pragma once
#include <Engine.Scene/SceneBase.hpp>

namespace hg::editor {
    class SceneFactory {
    public:
        [[nodiscard]] static uptr<engine::scene::SceneBase> createEditorScene();

        [[nodiscard]] static uptr<engine::scene::SceneBase> createGameScene();

        [[nodiscard]] static uptr<engine::scene::SceneBase> createPreviewScene();
    };
}
