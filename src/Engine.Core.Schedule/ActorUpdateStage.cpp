#include "ActorUpdateStage.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Core/World.hpp>
#include <Engine.Core/WorldContext.hpp>
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

                const auto* const engine = Engine::getEngine();
                const auto& ctxs = engine->getWorldContexts();

                for (const auto* const worldContext : ctxs) {

                    cref<sptr<World>> world = worldContext->getCurrentWorld();
                    if (not world) {
                        continue;
                    }

                    /**/

                    world->getScene()->broadcast({});
                    world->getScene()->update({});
                    world->getScene()->postprocess({});
                }

                return true;
            }
        )
    );
}

void ActorUpdateStage::dynamicDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) {
    // TODO:
}
