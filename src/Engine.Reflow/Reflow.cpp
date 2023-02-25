#include "Reflow.hpp"
#include "Module/Reflow.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Core/Module/CoreDependencies.hpp>
#include <Engine.Core/Module/DependencyKey.hpp>
#include <Engine.Core/Module/SubModuleDependency.hpp>
#include <Engine.Reflow.Schedule/ReflowPipeline.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Pipeline/CompositePipeline.hpp>

#include "Engine.Core/Engine.hpp"
#include "Window/WindowManager.hpp"

using namespace ember::engine::core;
using namespace ember::engine;
using namespace ember;

Reflow::Reflow(const non_owning_rptr<Engine> engine_) :
    SubModule(engine_),
    _dependencies(
        {
            SubModuleDependency {
                GraphicsDepKey,
                SubModuleOrder::eSuccessor,
                true
            }
        }
    ) {}

Reflow::~Reflow() = default;

core::DependencyKey Reflow::moduleKey() const noexcept {
    return ReflowDepKey;
}

cref<CompactSet<core::SubModuleDependency>> Reflow::dependencies() const noexcept {
    return _dependencies;
}

void Reflow::setup() {

    /**
     * Scheduling Pipelines
     */
    auto reflowPipeline = make_uptr<reflow::schedule::ReflowPipeline>();
    _engine->getScheduler()->getCompositePipeline()->addPipeline(_STD move(reflowPipeline));
}

void Reflow::start() {
    reflow::WindowManager::make();
}

void Reflow::stop() {
    reflow::WindowManager::destroy();
}

void Reflow::destroy() {}
