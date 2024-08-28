#include "ActorUpdateStage.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Scene/Scene.hpp>

using namespace hg::engine::core::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ActorUpdateStage::ActorUpdateStage(
	cref<StageIdentifier> identifier_,
	pipeline_handle_type pipeline_
) :
	PipelineStage(identifier_type::from(identifier_), pipeline_) {}

ActorUpdateStage::~ActorUpdateStage() = default;

void ActorUpdateStage::staticDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) {
	dispatcher_->enqueue(
		task::make_repetitive_task(
			[]() {

				const auto engine = Engine::getEngine();
				const auto& ctxs = engine->getUniverseContexts();

				for (const auto ctx : ctxs) {

					cref<sptr<Universe>> universe = ctx->getCurrentUniverse();
					if (not universe) {
						continue;
					}

					/**/

					universe->getScene()->broadcast({});
					universe->getScene()->update({});
					universe->getScene()->postprocess({});
				}

				return true;
			}
		)
	);
}

void ActorUpdateStage::dynamicDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) {
	// TODO:
}
