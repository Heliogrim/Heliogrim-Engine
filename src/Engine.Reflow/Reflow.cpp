#include "Reflow.hpp"
#include "Module/Reflow.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/CoreDependencies.hpp>
#include <Engine.Core/Module/DependencyKey.hpp>
#include <Engine.Core/Module/SubModuleDependency.hpp>
#include <Engine.Reflow.Schedule/ReflowPipeline.hpp>
#include <Engine.Reflow.Theming/Theming.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Pipeline/CompositePipeline.hpp>

#include "Window/WindowManager.hpp"

using namespace hg::engine::core;
using namespace hg::engine;
using namespace hg;

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
	),
	_theming() {}

Reflow::~Reflow() = default;

core::DependencyKey Reflow::moduleKey() const noexcept {
	return ReflowDepKey;
}

cref<CompactSet<core::SubModuleDependency>> Reflow::dependencies() const noexcept {
	return _dependencies;
}

void Reflow::setup() {

	_theming = make_uptr<reflow::theming::Theming>();

	/**
	 * Scheduling Pipelines
	 */
	auto reflowPipeline = make_uptr<reflow::schedule::ReflowPipeline>();
	_engine->getScheduler()->getCompositePipeline()->addPipeline(std::move(reflowPipeline));
}

void Reflow::start() {
	reflow::WindowManager::make();
}

void Reflow::stop() {
	reflow::WindowManager::destroy();
}

void Reflow::destroy() {
	_theming.reset();
}

ref<const reflow::theming::Theming> Reflow::getTheming() const noexcept {
	return *_theming;
}

ref<reflow::theming::Theming> Reflow::getTheming() noexcept {
	return *_theming;
}
