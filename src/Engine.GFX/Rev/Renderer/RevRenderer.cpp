#include "RevRenderer.hpp"

#include "../../Renderer/RenderPass.hpp"
#include "../../Renderer/RenderPipeline.hpp"
#include "../GraphicPass/RevDepthPass.hpp"
#include "../GraphicPass/RevFinalPass.hpp"
#include "../GraphicPass/RevLightPass.hpp"
#include "../GraphicPass/RevMainPass.hpp"
#include "../GraphicPass/RevProbePass.hpp"

#include "Engine.GFX/GraphicPass/GraphicPassMask.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RevRenderer::RevRenderer() :
    Renderer(),
    _pipeline(nullptr) {}

void RevRenderer::setup(cref<sptr<Device>> device_) {
    base_type::setup(device_);

    _pipeline = new RenderPipeline();

    _pipeline->defineGraphicPass(static_cast<u8>(GraphicPassMask::eDepthPass), new RevDepthPass(device_));
    //_pipeline->defineGraphicPass(static_cast<u8>(GraphicPassMask::eLightPass), new RevLightPass(device_));
    //_pipeline->defineGraphicPass(static_cast<u8>(GraphicPassMask::eProbePass), new RevProbePass(device_));
    _pipeline->defineGraphicPass(static_cast<u8>(GraphicPassMask::eMainPass), new RevMainPass(device_));
    _pipeline->defineGraphicPass(static_cast<u8>(GraphicPassMask::eFinalPass), new RevFinalPass(device_));

    _pipeline->setup();
}

void RevRenderer::destroy() {
    _pipeline->destroy();
    delete _pipeline;
}

const ptr<RenderPipeline> RevRenderer::pipeline() const noexcept {
    return _pipeline;
}
