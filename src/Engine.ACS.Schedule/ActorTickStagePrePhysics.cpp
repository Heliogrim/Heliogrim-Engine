#include "ActorTickStagePrePhysics.hpp"

#include <Engine.ACS.Schedule/ActorPipeline.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Input.Schedule/InputPipeline.hpp>

#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Level/Level.hpp>

using namespace hg::engine::acs::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ActorTickStagePrePhysics::ActorTickStagePrePhysics(
	cref<StageIdentifier> identifier_,
	pipeline_handle_type pipeline_
) :
	PipelineStage(identifier_type::from(identifier_), pipeline_) {}

ActorTickStagePrePhysics::~ActorTickStagePrePhysics() = default;

void ActorTickStagePrePhysics::staticDispatch(cref<StaticStageDispatcher> dispatcher_) {
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

void ActorTickStagePrePhysics::dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) {
	// TODO:
}

namespace hg::engine::core { class Timing; }
namespace hg::engine { class Input; }
namespace hg::engine::acs { class Registry; }

namespace {
	static constexpr graph::AccessKey Reads[] = { graph::AccessKey::of<engine::core::Timing>(), graph::AccessKey::of<engine::Input>() };
	static constexpr graph::AccessKey Writes[] = { graph::AccessKey::of<engine::acs::Registry>() };
}

graph::StageDecl ActorTickStagePrePhysics::decl() const noexcept {
	static const string_view after[] = { engine::core::schedule::CorePipeline::TickBegin, engine::input::schedule::InputPipeline::InputTick };
	return {
		.name = engine::acs::schedule::ActorPipeline::ActorTickPrePhysics,
		.reads = Reads,
		.writes = Writes,
		.lane = graph::Lane::eCritical,
		.after = after
	};
}
