#pragma once
#include <functional>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.Scene/Node/SceneNode.hpp>

#include "SceneHookFilter.hpp"
#include "SceneWalkerFilter.hpp"

namespace hg::engine::render::graph {
    class SceneWalker {
    public:
        using this_type = SceneWalker;

        using scene_model_type = gfx::scene::SceneModel;
        using scene_node_type = ptr<::hg::engine::scene::SceneNode<scene_model_type>>;

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

        _STD function<void(const ptr<const scene_model_type> model_)> _hook;

    public:
        void addWalkerFilter(mref<uptr<SceneWalkerFilter>> walkerFilter_);

        void addHookFilter(mref<uptr<SceneHookFilter>> hookFilter_);

        void setHook(mref<decltype(_hook)> hook_);

        template <typename Fn_>
        void setHook(Fn_&& fn_) {
            setHook(decltype(_hook) { _STD forward<Fn_>(fn_) });
        }

    protected:
        void stream(_STD span<ptr<scene_model_type>> models_) const noexcept;

        [[nodiscard]] bool operator()(const scene_node_type node_) const;

        [[nodiscard]] bool operator()(u32 batch_, const scene_node_type node_) const;

    public:
        ref<this_type> operator()(cref<gfx::scene::SceneView> sceneView_);

        ref<this_type> operator()(cref<gfx::scene::SceneView> sceneView_, u32 maxBatches_);

    public:
        [[nodiscard]] SceneWalkerFilter::checksum_type checksum() const noexcept;
    };
}
