#include "ActorHousekeepingStage.hpp"

#include <Engine.ACS/ActorModule.hpp>
#include <Engine.ACS/Registry.hpp>
#include <Engine.ACS/Handle/Epoch.hpp>
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
				registry->housekeeping().commit();

				const auto tables = registry->relocation().apply();
				if (tables.empty()) {
					return true;
				}

				for (const auto& ctx : engine->getUniverseContexts()) {
					const auto& universe = ctx->getCurrentUniverse();
					if (not universe || universe->getScene() == nullptr) {
						continue;
					}
					const auto scene = universe->getScene();
					for (size_t index = 0uLL; const auto system = scene->getSceneSystem(index); ++index) {
						for (const auto& table : tables) {
							system->relocate(table);
						}
					}
				}
				// Handles were rebound in place; the bump makes every ActorRef re-resolve once.
				::hg::engine::acs::Epoch::bump();

				return true;
			}
		)
	);
}

void ActorHousekeepingStage::dynamicDispatch(ref<const scheduler::DynamicStageDispatcher> dispatcher_) {}