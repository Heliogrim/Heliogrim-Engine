#include "GfxInit.hpp"

#include <Editor.Core/HeliogrimEditor.hpp>
#include <Heliogrim/UIComponent.hpp>
#include <Heliogrim/World.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/World.hpp>
#include <Engine.Core/WorldContext.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>

#include "Engine.GFX.Glow.UI/TestUI.hpp"

using namespace hg::editor::boot;
using namespace hg::engine;
using namespace hg;

void editor::boot::initGfx() {

    const auto* const engine = HeliogrimEditor::getEngine();
    const auto editor = HeliogrimEditor::getEditorSession();
    const auto game = HeliogrimEditor::getSession();

    /**/

    const auto* const editorSession = static_cast<ptr<core::Session>>(editor.unwrap().get());
    const auto* const editorWorld = editorSession->getWorldContext()->getCurrentWorld().get();

    const auto window = engine::reflow::WindowManager::get()->requestWindow(
        "Heliogrim Editor"sv,
        math::ivec2 { 1280, 720 },
        {},
        "Editor-Renderer"sv,
        editorWorld->getScene()
    );

    /**/

    testLoad(engine->getGraphics()->getCurrentDevice());
    buildTestUI(engine->getGraphics()->getCurrentDevice(), window.get());

    /**/

    auto* actor { CreateActor(editor) };
    auto* uic = editor.getActorInitializer().createComponent<UIComponent>(actor);

    uic->setWindow(window);

    GetWorld(editor).addActor(actor);
}
