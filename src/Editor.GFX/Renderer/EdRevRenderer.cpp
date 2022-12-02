#include "EdRevRenderer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX/Renderer/RenderPipeline.hpp>

#include "EdPostCompStage.hpp"

using namespace ember::editor::gfx;
using namespace ember::engine::gfx::glow::render;
using namespace ember;

EdRevRenderer::EdRevRenderer() :
    RevRenderer() {}

void EdRevRenderer::setup(cref<sptr<engine::gfx::Device>> device_) {
    RevRenderer::setup(device_);
}

void EdRevRenderer::destroy() {
    RevRenderer::destroy();
}

void EdRevRenderer::registerStages() {
    RevRenderer::registerStages();

    /**/

    const auto pipe { getOrCreatePipeline() };

    const auto edPost { make_sptr<EdPostCompStage>() };
    edPost->pushDependency({
        pipe->stages().back().get(),
        engine::gfx::render::RenderStageOrder::ePredecessor,
        true
    });
    pipe->push(edPost);
}
