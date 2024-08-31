#include "ActorPipeline.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Input.Schedule/InputPipeline.hpp>

#include "ActorTickStage.hpp"

using namespace hg::engine::acs::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ActorPipeline::ActorPipeline() :
	StagePipeline(identifier_type::from("::Core::ActorPipeline")) {}

ActorPipeline::~ActorPipeline() = default;

void ActorPipeline::mount(ref<StageRegister> register_) {

	const auto actorTick = register_.registerStage(
		make_uptr<ActorTickStage>(ActorTick, this)
	);

	/**/

	_orderedStages.push_back(actorTick);
}

void ActorPipeline::declareDependencies(
	cref<StageRegister> register_,
	ref<CompactSet<StageDependency>> collection_
) {
	const auto* const beginTick = register_.getStage(core::schedule::CorePipeline::TickBegin);
	const auto* const inputTick = register_.getStage(input::schedule::InputPipeline::InputTick);

	/**/

	const auto* const actorTick = _orderedStages.front();

	/**/

	collection_.insert(
		StageDependency {
			{ beginTick, inputTick },
			this,
			actorTick
		}
	);
}

void ActorPipeline::dismount(ref<StageRegister> register_) {

	register_.removeStage(ActorTick);

	/**/

	_orderedStages.clear();
}

bool ActorPipeline::isSkippable() const noexcept {
	return false;
}
