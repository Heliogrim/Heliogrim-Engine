#include "Editor.hpp"
#include "Module/Editor.hpp"

#include <Editor.Action/ActionManager.hpp>
#include <Engine.Core/Module/CoreDependencies.hpp>
#include <Engine.Core/Module/DependencyKey.hpp>
#include <Engine.Core/Module/SubModuleDependency.hpp>
#include <Engine.Reflow/Module/Reflow.hpp>

#include "Boot/AssetInit.hpp"
#include "Boot/RenderInit.hpp"
#include "Boot/UniverseInit.hpp"

using namespace hg::editor;
using namespace hg::engine::core;
using namespace hg;

Editor::Editor(const non_owning_rptr<engine::Engine> engine_) :
	SubModule(engine_),
	_dependencies(
		{
			SubModuleDependency {
				.key = ActionDepKey,
				.order = SubModuleOrder::eSuccessor,
				.required = true
			},
			SubModuleDependency {
				.key = AssetsDepKey,
				.order = SubModuleOrder::eSuccessor,
				.required = true
			},
			SubModuleDependency {
				.key = engine::ReflowDepKey,
				.order = SubModuleOrder::eSuccessor,
				.required = true
			}
		}
	) {}

Editor::~Editor() = default;

DependencyKey Editor::moduleKey() const noexcept {
	return EditorDepKey;
}

cref<CompactSet<engine::core::SubModuleDependency>> Editor::dependencies() const noexcept {
	return _dependencies;
}

void Editor::setup() {}

void Editor::start() {
	boot::initRender();
	boot::initAssets();
	boot::initEditorUniverse();
	boot::initPrimaryUniverse();
}

void Editor::stop() {}

void Editor::destroy() {}
