#include "GfxInit.hpp"

#include <Editor.Core/EmberEditor.hpp>
#include <Ember/UIComponent.hpp>
#include <Ember/World.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/World.hpp>
#include <Engine.Core/WorldContext.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>
#include <Engine.Scene/RevScene.hpp>

#include "Engine.GFX.Glow.UI/TestUI.hpp"

using namespace ember::editor::boot;
using namespace ember::engine;
using namespace ember;

void editor::boot::initGfx() {

    const auto* const engine = EmberEditor::getEngine();
    const auto editor = EmberEditor::getEditorSession();
    const auto game = EmberEditor::getSession();

    /**/

    const auto* const editorSession = static_cast<ptr<core::Session>>(editor.unwrap().get());
    const auto* const editorWorld = editorSession->getWorldContext()->getCurrentWorld().get();

    const auto window = engine::reflow::WindowManager::get()->requestWindow(
        "Ember Editor"sv,
        math::ivec2 { 1280, 720 },
        {},
        static_cast<ptr<scene::RevScene>>(editorWorld->getScene())
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
