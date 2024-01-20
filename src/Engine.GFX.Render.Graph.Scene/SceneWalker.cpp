#include "SceneWalker.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

SceneWalker::SceneWalker(
    mref<Vector<uptr<SceneWalkerFilter>>> walkFilter_,
    mref<Vector<uptr<SceneHookFilter>>> hookFilter_
) :
    _walkFilter(_STD move(walkFilter_)),
    _hookFilter(_STD move(hookFilter_)),
    _hook() {}

void SceneWalker::setHook(mref<decltype(_hook)> hook_) {
    _hook = _STD move(hook_);
}

void SceneWalker::stream(std::span<ptr<RenderSceneSystemModel>> models_) const noexcept {

    for (const auto& model : models_) {

        auto filterIt = _hookFilter.begin();
        const auto filterEnd = _hookFilter.end();
        while (filterIt != filterEnd && (**filterIt++)(model)) {
            // __noop;
        }

        _hook(model);
    }
}


bool SceneWalker::operator()(const void* node_) const {

    #ifdef _DEBUG
    if (not _hook) {
        __debugbreak();
        return false;
    }
    #endif

    auto filterIt = _walkFilter.begin();
    const auto filterEnd = _walkFilter.end();
    while (filterIt != filterEnd && (**filterIt++)(node_)) {
        // __noop;
    }

    if (filterIt != filterEnd) {
        return false;
    }

    //stream({ node_->elements(), node_->exclusiveSize() });
    return true;
}

bool SceneWalker::operator()([[maybe_unused]] u32 batch_, const void* node_) const {
    return (*this)(node_);
}

ref<SceneWalker::this_type> SceneWalker::operator()(cref<gfx::scene::SceneView> sceneView_) {

    #ifdef _DEBUG
    if (not _hook) {
        __debugbreak();
        return *this;
    }
    #endif

    /**/

    for (const auto& walkFilter : _walkFilter) {
        walkFilter->update(sceneView_);
    }

    /**/

    //sceneView_.getScene()->renderGraph()->traversal(
    //    [this](auto&&... args_) {
    //        return this->operator()(_STD forward<decltype(args_)>(args_)...);
    //    }
    //);
    return *this;
}

ref<SceneWalker::this_type> SceneWalker::operator()(cref<gfx::scene::SceneView> sceneView_, u32 maxBatches_) {

    if (maxBatches_ <= 1uL) {
        return (*this)(sceneView_);
    }

    #ifdef _DEBUG
    if (not _hook) {
        __debugbreak();
        return *this;
    }
    #endif

    /**/

    for (const auto& walkFilter : _walkFilter) {
        walkFilter->update(sceneView_);
    }

    /**/

    //sceneView_.getScene()->renderGraph()->traversalBatched(
    //    maxBatches_,
    //    [this](auto&&... args_) {
    //        return this->operator()(_STD forward<decltype(args_)>(args_)...);
    //    }
    //);
    return *this;
}

SceneWalkerFilter::checksum_type SceneWalker::checksum() const noexcept {

    SceneWalkerFilter::checksum_type combined {};
    for (const auto& filter : _walkFilter) {
        hash::hashCombine(combined, filter->checksum());
    }

    return combined;
}
