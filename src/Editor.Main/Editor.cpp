#include "Editor.hpp"
#include "Module/Editor.hpp"

#include <Engine.Core/Module/CoreDependencies.hpp>
#include <Engine.Core/Module/DependencyKey.hpp>
#include <Engine.Core/Module/SubModuleDependency.hpp>
#include <Engine.GFX.Glow.UI/TestUI.hpp>
#include <Engine.Reflow/Module/Reflow.hpp>

#include "Boot/AssetInit.hpp"
#include "Boot/GfxInit.hpp"
#include "Boot/WorldInit.hpp"

using namespace hg::editor;
using namespace hg::engine::core;
using namespace hg;

Editor::Editor(const non_owning_rptr<engine::Engine> engine_) :
    SubModule(engine_),
    _dependencies(
        {
            SubModuleDependency {
                AssetsDepKey,
                SubModuleOrder::eSuccessor,
                true
            },
            SubModuleDependency {
                engine::ReflowDepKey,
                SubModuleOrder::eSuccessor,
                true
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
    boot::initAssets();
    boot::initEditorWorld();
    boot::initPrimaryWorld();
    boot::initGfx();
}

void Editor::stop() {
    destroyLoaded();
}

void Editor::destroy() {}
