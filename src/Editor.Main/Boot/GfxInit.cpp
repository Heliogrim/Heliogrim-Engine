#include "GfxInit.hpp"

#include <Editor.Core/EmberEditor.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Glow.UI/Renderer/UIRenderer.hpp>

#include <Engine.GFX.Scene/RenderSceneManager.hpp>
#include <Engine.GFX/RenderTarget.hpp>

using namespace ember::editor::boot;
using namespace ember::engine;
using namespace ember;

void editor::boot::initGfx() {

    const auto* const engine { EmberEditor::getEngine() };
    const auto editor { EmberEditor::getEditorSession() };
    const auto game { EmberEditor::getSession() };

    const auto* const gfx { engine->getGraphics() };
    auto* const gfxScenes { gfx->getSceneManager() };

    const auto renderTarget = make_sptr<gfx::RenderTarget>();
    renderTarget->use(gfx->getCurrentDevice());
    renderTarget->use(gfx->getCurrentSwapchain());
    renderTarget->use(gfx->getRenderer(AssocKey<string>::from("UIRenderer")).get());

    gfxScenes->addPrimaryTarget(renderTarget);
}
