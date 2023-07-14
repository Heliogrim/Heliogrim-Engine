#include <Heliogrim/Inbuilt.hpp>

#include "RenderPipeline.hpp"
#include "State/State.hpp"
#include "Stage/Stage.hpp"
#include "Stage/SubpassStage.hpp"

#include "Stage/AccelSubpassStage.hpp"
#include "Stage/MaterialSubpassStage.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg::engine::gfx::render;
using namespace hg;

auto build() {

    Vector<smr<Stage>> stages {};

    // Early-Z
    stages.push_back(make_smr<AccelSubpassStage>());

    // Material
    stages.push_back(make_smr<MaterialSubpassStage>());
    stages.back()->setExecStateCnd(&(**(stages.begin() + 0)).getExecState().state);

    // Lighting
    stages.push_back(make_smr<AccelSubpassStage>());
    stages.back()->setExecStateCnd(&(**(stages.begin() + 1)).getExecState().state);

    return stages;
}

void test_render_pipe() {

    RenderPipeline pipeline {};
    uptr<State> state = make_uptr<State>(nullptr, engine::gfx::memory::LocalPooledAllocator { nullptr });

    pipeline.setStages(build());

    pipeline.regSubState(state.get());
    pipeline.alloc(state.get());

    static bool interrupt = false;
    while (not interrupt) {
        pipeline.reset();
        auto next = pipeline(_STD move(state));

        next.first == RenderPipelineResult::eSuccess;
        next.first == RenderPipelineResult::eFailed;
        state = _STD move(next.second);
    }

    pipeline.free(state.get());
}
