#include "ActorHousekeepingStage.hpp"

#include <Engine.ACS/ActorModule.hpp>
#include <Engine.ACS/Registry.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Scene/SceneBase.hpp>
#include <Engine.Scene/SceneSystem.hpp>

using namespace hg::engine::acs::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ActorHousekeepingStage::ActorHousekeepingStage(
	ref<const StageIdentifier> identifier_,
	pipeline_handle_type pipeline_
) :
	PipelineStage(identifier_type::from(identifier_), pipeline_) {}

ActorHousekeepingStage::~ActorHousekeepingStage() = default;

void ActorHousekeepingStage::staticDispatch(ref<const scheduler::StaticStageDispatcher> dispatcher_) {
	dispatcher_.enqueue(
		task::make_repetitive_task(
			[]() {
				const auto engine = Engine::getEngine();
				const auto registry = engine->getActors()->getRegistry();
				registry->housekeep(
					[&engine](ref<const engine::acs::RelocationTable> table_) {
						for (const auto& ctx : engine->getUniverseContexts()) {
							const auto& universe = ctx->getCurrentUniverse();
							if (not universe || universe->getScene() == nullptr) {
								continue;
							}
							const auto scene = universe->getScene();
							scene->forEachSystem([&table_](ref<scene::SceneSystemBase> system_) { system_.relocate(table_); });
						}
					}
				);

				return true;
			}
		)
	);
}

void ActorHousekeepingStage::dynamicDispatch(ref<const scheduler::DynamicStageDispatcher> dispatcher_) {}
