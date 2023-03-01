#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/CompactArray.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Utils/_CTRL.hpp>

namespace hg::engine::scene {
    class IRenderScene;
}

namespace hg::engine::gfx {
    class RenderTarget;
}

namespace hg::engine::gfx::scene {
    class RenderSceneManager final {
    public:
        using this_type = RenderSceneManager;

        using RenderScene = ::hg::engine::scene::IRenderScene;

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

        template <typename Selector_>
        void selectPrimaryTargets(
            _Inout_ ref<CompactArray<sptr<RenderTarget>>> targets_,
            Selector_&& selector_
        ) const noexcept(_STD is_nothrow_invocable_v<Selector_, cref<sptr<RenderTarget>>>) {
            for (const auto& target : _primaryTargets) {
                if (selector_(target)) {
                    targets_.push_back(target);
                }
            }
        }

        template <typename Selector_>
        void selectSecondaryTargets(
            _Inout_ ref<CompactArray<sptr<RenderTarget>>> targets_,
            Selector_&& selector_
        ) const noexcept(_STD is_nothrow_invocable_v<Selector_, cref<sptr<RenderTarget>>>) {
            for (const auto& target : _secondaryTargets) {
                if (selector_(target)) {
                    targets_.push_back(target);
                }
            }
        }

        template <typename Selector_>
        void selectTargets(
            _Inout_ ref<CompactArray<sptr<RenderTarget>>> targets_,
            Selector_&& selector_
        ) const noexcept(_STD is_nothrow_invocable_v<Selector_, cref<sptr<RenderTarget>>>) {
            selectPrimaryTargets<Selector_>(targets_, _STD forward<Selector_>(selector_));
            selectSecondaryTargets<Selector_>(targets_, _STD forward<Selector_>(selector_));
        }
    };
}
