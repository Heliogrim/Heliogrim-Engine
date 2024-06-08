#include "SubModuleInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Editor.GFX.IO/Module/EditorGraphicsIo.hpp>
#include <Editor.UI.Main/EditorUI.hpp>
#include <Engine.Accel/Accel.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Reflow.hpp>

#include "../Editor.hpp"

using namespace hg::editor::boot;
using namespace hg;

void editor::boot::preInitSubModules() {

	const auto engine = EditorEngine::getEngine();
	auto& modules = engine->getModules();

	/**/

	auto result = modules.addSubModule(make_uptr<engine::Accel>(engine.get()));
	::hg::assertrt(result == engine::core::DependencyValidationResult::eSuccess);

	result = modules.addSubModule(make_uptr<engine::Reflow>(engine.get()));
	::hg::assertrt(result == engine::core::DependencyValidationResult::eSuccess);

	/**/

	result = modules.addSubModule(make_uptr<EditorGraphicsIo>(engine));
	::hg::assertrt(result == engine::core::DependencyValidationResult::eSuccess);

	result = modules.addSubModule(make_uptr<Editor>(engine.get()));
	::hg::assertrt(result == engine::core::DependencyValidationResult::eSuccess);

	result = modules.addSubModule(make_uptr<EditorUI>(*engine.get()));
	::hg::assertrt(result == engine::core::DependencyValidationResult::eSuccess);
}
