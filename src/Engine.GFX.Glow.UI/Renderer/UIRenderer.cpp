#include "UIRenderer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.Renderer/RenderPipeline_Deprecated.hpp>

#include "UiMainStage.hpp"

using namespace hg::engine::gfx::glow::ui::render;
using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

UiRenderer::UiRenderer() :
    Renderer_Deprecated() {}

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
    Renderer_Deprecated::setup(device_);
}

void UiRenderer::destroy() {}
