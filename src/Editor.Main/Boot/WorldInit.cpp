#include "WorldInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/World.hpp>
#include <Engine.Scene/SceneFactory.hpp>

#include "Engine.Core/WorldContext.hpp"

using namespace ember::editor::boot;
using namespace ember::engine;
using namespace ember;

void editor::boot::initEditorWorld() {

    auto* const engine { EditorEngine::getEngine() };
    const auto* const editor { engine->getEditorSession() };

    /**/

    auto scene { scene::SceneFactory::createDefaultScene() };
    const auto world { make_sptr<core::World>(_STD move(scene)) };

    engine->addWorld(world);
    editor->getWorldContext()->setCurrentWorld(world);
}

void editor::boot::initPrimaryWorld() {

    auto* const engine { EditorEngine::getEngine() };
    const auto* const primary { engine->getPrimaryGameSession() };

    /**/

    auto scene { scene::SceneFactory::createDefaultScene() };
    const auto world { make_sptr<core::World>(_STD move(scene)) };

    engine->addWorld(world);
    primary->getWorldContext()->setCurrentWorld(world);
}
