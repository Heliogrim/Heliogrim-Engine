#include "UIRenderer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX/Renderer/RenderPipeline.hpp>

#include "UiMainStage.hpp"

using namespace ember::engine::gfx::glow::ui::render;
using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

UiRenderer::UiRenderer() :
    Renderer() {}

void UiRenderer::setup(cref<sptr<Device>> device_) {

    const auto pipe { getOrCreatePipeline() };

    /**
     * Create Stages
     */
    const auto mainStage { make_sptr<UiMainStage>() };

    /**
     * Declare Dependencies
     */

    /**
     * Push Stages
     */
    pipe->push(mainStage);

    /**
     * Setup Pipeline & Store Device
     */
    Renderer::setup(device_);
}

void UiRenderer::destroy() {}
