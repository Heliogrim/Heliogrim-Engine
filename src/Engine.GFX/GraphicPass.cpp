#include "GraphicPass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::gfx;
using namespace ember;

#include <atomic>
inline static _STD atomic_flag created;

GraphicPass::GraphicPass(cref<GraphicPassMask> mask_) :
    _mask(mask_) {}

void GraphicPass::process(cref<scene::SceneGraph> graph_, ref<CommandBatch> batch_) {

    SCOPED_STOPWATCH

    ptr<ModelPassProcessor> processor = this->processor();
    if (processor && created.test(_STD memory_order::acquire)) {
        struct ConsumerWrapper {

            ref<ModelPassProcessor> obj;

            bool (ModelPassProcessor::*fnc)(cref<scene::SceneNode>);

            [[nodiscard]] bool operator()(cref<scene::SceneNode> node_) const noexcept {
                return (obj.*fnc)(node_);
            }
        };

        const ConsumerWrapper consumer { *processor, &ModelPassProcessor::operator() };
        graph_.traversal(consumer);
    }
    _pipeline.process(processor, batch_);
}

GraphicPassMask GraphicPass::mask() const noexcept {
    return _mask;
}

GraphicPassMask GraphicPass::mask() noexcept {
    return _mask;
}

cref<GraphicPassPipeline> GraphicPass::pipeline() const noexcept {
    return _pipeline;
}
