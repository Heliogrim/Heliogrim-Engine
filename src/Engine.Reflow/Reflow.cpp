#include "Reflow.hpp"

#include <Engine.Reflow.Theming/Theming.hpp>

#include "DataWatcher.hpp"
#include "Module/Reflow.hpp"
#include "Window/WindowManager.hpp"

import Engine.Common;
import Engine.Core;
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

ref<const reflow::DataWatcher> Reflow::getDataWatcher() const noexcept {
	return *_watcher;
}

ref<reflow::DataWatcher> Reflow::getDataWatcher() noexcept {
	return *_watcher;
}
