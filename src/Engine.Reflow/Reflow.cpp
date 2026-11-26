#include "Reflow.hpp"

#include <Engine.Reflow.Theming/Theming.hpp>

#include "DataWatcher.hpp"
#include "Module/Reflow.hpp"
#include "Window/WindowManager.hpp"

import Engine.Common;
import Engine.Core;
import Engine.Core.Schedule;
import Engine.Reflow.Schedule;
import Engine.Scheduler;

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
	_theming(),
	_watcher() {}

Reflow::~Reflow() = default;

core::DependencyKey Reflow::moduleKey() const noexcept {
	return ReflowDepKey;
}

cref<CompactSet<core::SubModuleDependency>> Reflow::dependencies() const noexcept {
	return _dependencies;
}

void Reflow::setup() {

	_theming = make_uptr<reflow::theming::Theming>();
	_watcher = make_uptr<reflow::DataWatcher>();

	/**
	 * Scheduling Pipelines
	 */
	auto reflowPipeline = make_uptr<reflow::schedule::ReflowPipeline>();
	Scheduler::get()->getCompositePipeline()->addPipeline(std::move(reflowPipeline));
}

void Reflow::start() {
	reflow::WindowManager::make();
}

void Reflow::stop() {
	// Only called from the stop task (a fiber, `Signal::wait` parks it), never from a frame task.
	// Posts in stop phases 1 and 2 always run, as the pipeline stops in phase 3.
	scheduler::Signal destroyed {};
	scheduler::execOnStage(
		[&destroyed] {
			reflow::WindowManager::destroy();
			destroyed.set();
		},
		core::schedule::CorePipeline::Teardown
	);
	destroyed.wait();
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

ref<const reflow::DataWatcher> Reflow::getDataWatcher() const noexcept {
	return *_watcher;
}

ref<reflow::DataWatcher> Reflow::getDataWatcher() noexcept {
	return *_watcher;
}
