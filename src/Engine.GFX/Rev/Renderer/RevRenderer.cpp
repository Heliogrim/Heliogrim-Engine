#include "RevRenderer.hpp"

#include <Engine.GFX/Renderer/RenderPipeline.hpp>

#include "RevDepthStage.hpp"
#include "RevFinalStage.hpp"
#include "RevLightStage.hpp"
#include "RevMainStage.hpp"
#include "RevProbeStage.hpp"

using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

RevRenderer::RevRenderer() :
    Renderer() {}

void RevRenderer::setup(cref<sptr<Device>> device_) {

    const auto pipe { getOrCreatePipeline() };

    /**
     * Create Stages
     */
    const auto depthStage { make_sptr<RevDepthStage>() };
    const auto lightStage { make_sptr<RevLightStage>() };
    const auto probeStage { make_sptr<RevProbeStage>() };
    const auto mainStage { make_sptr<RevMainStage>() };
    const auto finalStage { make_sptr<RevFinalStage>() };

    /**
     * Declare Dependencies
     */
    lightStage->pushDependency({
        depthStage.get(),
        RenderStageOrder::ePredecessor,
        true
    });

    probeStage->pushDependency({
        depthStage.get(),
        RenderStageOrder::ePredecessor,
        true
    });

    mainStage->pushDependency({
        depthStage.get(),
        RenderStageOrder::ePredecessor,
        true
    });

    mainStage->pushDependency({
        lightStage.get(),
        RenderStageOrder::ePredecessor,
        true
    });

    mainStage->pushDependency({
        probeStage.get(),
        RenderStageOrder::ePredecessor,
        true
    });

    finalStage->pushDependency({
        mainStage.get(),
        RenderStageOrder::ePredecessor,
        true
    });

    /**
     * Push Stages
     */
    pipe->push(depthStage);
    pipe->push(mainStage);
    pipe->push(finalStage);

    /**
     * Setup Pipeline & Store Device
     */
    Renderer::setup(device_);
}

void RevRenderer::destroy() {}
