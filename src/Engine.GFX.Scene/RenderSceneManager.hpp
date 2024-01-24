#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/CompactArray.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Utils/_CTRL.hpp>

namespace hg {
    class CameraComponent;
}

namespace hg::engine::render {
    class Renderer;
}

namespace hg::engine::scene {
    class SceneBase;
}

namespace hg::engine::gfx {
    class RenderTarget;
    class Swapchain;
    class Surface;
}

namespace hg::engine::gfx::scene {
    class SceneView;
}

namespace hg::engine::gfx::scene {
    class RenderSceneManager final {
    public:
        using this_type = RenderSceneManager;

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
        CompactSet<ptr<engine::scene::SceneBase>> _renderScenes;
        _CTRL_OBJ(_ctrl);

    private:
        void injectSceneHooks(const ptr<engine::scene::SceneBase> renderableScene_);

    public:
        void registerScene(const ptr<engine::scene::SceneBase> renderableScene_);

        bool unregisterScene(const ptr<engine::scene::SceneBase> renderableScene_);

    private:
        CompactSet<smr<RenderTarget>> _primaryTargets;
        DenseMap<smr<Swapchain>, smr<RenderTarget>> _mappedRenderTargets;

    public:
        void addPrimaryTarget(mref<smr<RenderTarget>> renderTarget_);

        void dropPrimaryTarget(cref<smr<RenderTarget>> renderTarget_);

    public:
        void transitionToSceneView(
            mref<smr<Swapchain>> targetKey_,
            mref<smr<SceneView>> nextView_
        );

        void transitionToTarget(
            mref<smr<Swapchain>> from_,
            mref<smr<Swapchain>> toSwapChain_,
            nmpt<Surface> toSurface_
        );

    public:
        void selectInvokeTargets(_Inout_ ref<CompactSet<smr<RenderTarget>>> targets_) const noexcept;

        template <typename Selector_>
        void selectPrimaryTargets(
            _Inout_ ref<CompactArray<smr<RenderTarget>>> targets_,
            Selector_&& selector_
        ) const noexcept(std::is_nothrow_invocable_v<Selector_, cref<smr<RenderTarget>>>) {
            for (const auto& target : _primaryTargets) {
                if (selector_(target)) {
                    targets_.emplace_back(clone(target));
                }
            }
        }

        template <typename Selector_>
        void selectTargets(
            _Inout_ ref<CompactArray<smr<RenderTarget>>> targets_,
            Selector_&& selector_
        ) const noexcept(std::is_nothrow_invocable_v<Selector_, cref<smr<RenderTarget>>>) {
            selectPrimaryTargets<Selector_>(targets_, std::forward<Selector_>(selector_));
        }
    };
}
