#include "SubModuleInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Reflow.hpp>

#include "../Editor.hpp"

using namespace hg::editor::boot;
using namespace hg;

void editor::boot::preInitSubModules() {

    auto* const engine = EditorEngine::getEngine();
    auto& modules = engine->getModules();

    /**/

    auto result = modules.addSubModule(make_uptr<engine::Reflow>(engine));
    assert(result == engine::core::DependencyValidationResult::eSuccess);

    result = modules.addSubModule(make_uptr<Editor>(engine));
    assert(result == engine::core::DependencyValidationResult::eSuccess);
}
