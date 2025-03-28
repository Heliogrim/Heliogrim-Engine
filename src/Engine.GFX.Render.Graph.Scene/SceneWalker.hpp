#pragma once
#include <functional>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>

#include "SceneHookFilter.hpp"
#include "SceneWalkerFilter.hpp"

namespace hg::engine::render {
    class RenderSceneSystemModel;
}

namespace hg::engine::render::graph {
    class SceneWalker {
    public:
        using this_type = SceneWalker;

    public:
        SceneWalker() noexcept = default;

        SceneWalker(
            mref<Vector<uptr<SceneWalkerFilter>>> walkFilter_,
            mref<Vector<uptr<SceneHookFilter>>> hookFilter_
        );

        SceneWalker(mref<this_type> other_) noexcept = default;

        SceneWalker(cref<this_type>) = delete;

        ~SceneWalker() noexcept = default;

    public:
        ref<this_type> operator=(mref<this_type> other_) noexcept = default;

        ref<this_type> operator=(cref<this_type>) = default;

    private:
        Vector<uptr<SceneWalkerFilter>> _walkFilter;
        Vector<uptr<SceneHookFilter>> _hookFilter;

        std::function<void(const ptr<const RenderSceneSystemModel> model_)> _hook;

    public:
        void addWalkerFilter(mref<uptr<SceneWalkerFilter>> walkerFilter_);

        void addHookFilter(mref<uptr<SceneHookFilter>> hookFilter_);

        void setHook(mref<decltype(_hook)> hook_);

        template <typename Fn_>
        void setHook(Fn_&& fn_) {
            setHook(decltype(_hook) { std::forward<Fn_>(fn_) });
        }

    protected:
        void stream(std::span<ptr<RenderSceneSystemModel>> models_) const noexcept;

        [[nodiscard]] bool operator()(const void* node_) const;

        [[nodiscard]] bool operator()(u32 batch_, const void* node_) const;

    public:
        ref<this_type> operator()(cref<gfx::scene::SceneView> sceneView_);

        ref<this_type> operator()(cref<gfx::scene::SceneView> sceneView_, u32 maxBatches_);

    public:
        [[nodiscard]] SceneWalkerFilter::checksum_type checksum() const noexcept;
    };
}
