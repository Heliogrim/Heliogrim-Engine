#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Utils/_CTRL.hpp>

namespace ember::engine::scene {
    class IRenderScene;
}

namespace ember::engine::gfx {
    class RenderTarget;
}

namespace ember::engine::gfx::scene {

    class RenderSceneManager final {
    public:
        using this_type = RenderSceneManager;

        using RenderScene = ::ember::engine::scene::IRenderScene;

    private:
        /**
         * Singleton Instance
         */
        static uptr<RenderSceneManager> _instance;

    public:
        [[nodiscard]] static non_owning_rptr<this_type> get() noexcept;

        static non_owning_rptr<this_type> make();

        static void destroy();

    private:
        RenderSceneManager();

    public:
        RenderSceneManager(cref<this_type>) = delete;

        RenderSceneManager(mref<this_type>) noexcept = delete;

        ~RenderSceneManager();

    private:
        CompactSet<ptr<RenderScene>> _renderScenes;
        _CTRL_OBJ(_ctrl);

    private:
        void injectSceneHooks(const ptr<RenderScene> renderScene_);

    public:
        void registerScene(const ptr<RenderScene> renderScene_);

        bool unregisterScene(const ptr<RenderScene> renderScene_);

    private:
        CompactSet<sptr<RenderTarget>> _primaryTargets;
        CompactSet<sptr<RenderTarget>> _secondaryTargets;

    public:
        void addPrimaryTarget(cref<sptr<RenderTarget>> renderTarget_);

        void dropPrimaryTarget(const sptr<RenderTarget> renderTarget_);

        void addSecondaryTarget(cref<sptr<RenderTarget>> renderTarget_);

        void dropSecondaryTarget(const sptr<RenderTarget> renderTarget_);

    public:
        void selectInvokeTargets(_Inout_ ref<CompactSet<sptr<RenderTarget>>> targets_) const noexcept;
    };

}
