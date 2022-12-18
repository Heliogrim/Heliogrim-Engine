#include "Reflow.hpp"

#include <Engine.Core/Module/DependencyKey.hpp>
#include <Engine.Core/Module/CoreDependencies.hpp>
#include <Engine.Core/Module/SubModuleDependency.hpp>

#include "Window/WindowManager.hpp"
#include "Module/Reflow.hpp"

using namespace ember::engine::core;
using namespace ember::engine;
using namespace ember;

Reflow::Reflow(const non_owning_rptr<Engine> engine_) :
    SubModule(engine_),
    _dependencies({
        SubModuleDependency {
            GraphicsDepKey,
            SubModuleOrder::eSuccessor,
            true
        }
    }) {}

Reflow::~Reflow() = default;

core::DependencyKey Reflow::moduleKey() const noexcept {
    return ReflowDepKey;
}

cref<CompactSet<core::SubModuleDependency>> Reflow::dependencies() const noexcept {
    return _dependencies;
}

void Reflow::setup() {}

void Reflow::start() {
    reflow::WindowManager::make();
}

void Reflow::stop() {
    reflow::WindowManager::destroy();
}

void Reflow::destroy() {}
