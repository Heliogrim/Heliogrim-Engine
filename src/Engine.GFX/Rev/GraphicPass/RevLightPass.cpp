#include "RevLightPass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::gfx;
using namespace ember;

RevLightPass::RevLightPass(cref<sptr<Device>> device_) :
    GraphicPass(device_, GraphicPassMask::eLightPass) {}

void RevLightPass::setup() {

    SCOPED_STOPWATCH

}

void RevLightPass::destroy() {
    auto& stages = _pipeline.stages();
    for (auto it { stages.rbegin() }; it != stages.rend(); ++it) {

        auto& stage { *it };

        stage->destroy();
        delete stage;
        stage = nullptr;
    }

    stages.clear();
}

ptr<GraphicPassModelProcessor> RevLightPass::processor() noexcept {
    return nullptr;
}
