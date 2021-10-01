#include "ProbePass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::gfx;
using namespace ember;

ProbePass::ProbePass() :
    GraphicPass(GraphicPassMask::eProbePass) {}

void ProbePass::setup() {

    SCOPED_STOPWATCH

}

void ProbePass::destroy() {}

ptr<ModelPassProcessor> ProbePass::processor() noexcept {
    return nullptr;
}
