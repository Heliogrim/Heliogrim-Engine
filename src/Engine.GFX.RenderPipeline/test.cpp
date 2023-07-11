#include <Heliogrim/Inbuilt.hpp>

#include "RenderPipeline.hpp"
#include "State/State.hpp"
#include "Stage/Stage.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg::engine::gfx::render;
using namespace hg;

void test() {

    RenderPipeline pipeline {};
    uptr<State> state {};

    pipeline.regSubState(state.get());
    pipeline.alloc(state.get());

    while (true) {
        auto next = pipeline(_STD move(state));

        next.first == RenderPipelineResult::eSuccess;
        next.first == RenderPipelineResult::eFailed;
        state = _STD move(next.second);

        break;
    }

    pipeline.free(state.get());
}
