#pragma once
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Scene;
#endif

namespace hg::editor {
    class SceneFactory {
    public:
        [[nodiscard]] static uptr<engine::scene::SceneBase> createEditorScene();

        [[nodiscard]] static uptr<engine::scene::SceneBase> createGameScene();

        [[nodiscard]] static uptr<engine::scene::SceneBase> createPreviewScene();
    };
}
