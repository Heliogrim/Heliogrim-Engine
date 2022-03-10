#include "RevProbePass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::gfx;
using namespace ember;

RevProbePass::RevProbePass(cref<sptr<Device>> device_) :
    GraphicPass(device_, GraphicPassMask::eProbePass) {}

void RevProbePass::setup() {

    SCOPED_STOPWATCH

}

void RevProbePass::destroy() {
    auto& stages = _pipeline.stages();
    for (auto it { stages.rbegin() }; it != stages.rend(); ++it) {

        auto& stage { *it };

        stage->destroy();
        delete stage;
        stage = nullptr;
    }

    stages.clear();
}

ptr<GraphicPassModelProcessor> RevProbePass::processor() noexcept {
    return nullptr;
}
