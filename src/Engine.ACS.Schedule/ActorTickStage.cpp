#include "ActorTickStage.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Level/Level.hpp>

using namespace hg::engine::acs::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ActorTickStage::ActorTickStage(
	cref<StageIdentifier> identifier_,
	pipeline_handle_type pipeline_
) :
	PipelineStage(identifier_type::from(identifier_), pipeline_) {}

ActorTickStage::~ActorTickStage() = default;

void ActorTickStage::staticDispatch(cref<StaticStageDispatcher> dispatcher_) {
	dispatcher_.enqueue(
		task::make_repetitive_task(
			[]() {

				const auto engine = Engine::getEngine();
				const auto& ctxs = engine->getUniverseContexts();

				for (const auto& ctx : ctxs) {

					const auto& universe = ctx->getCurrentUniverse();
					if (not universe) {
						continue;
					}

					/**/

					for (const auto& level : universe->getLevels()) {
						for (const auto& actor : level->getActors()) {
							// TODO: Tick actor
							std::ignore = actor->getComponents();
						}
					}
				}

				return true;
			}
		)
	);
}

void ActorTickStage::dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) {
	// TODO:
}
