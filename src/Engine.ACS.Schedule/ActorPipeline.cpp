#include "ActorPipeline.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.GFX.Schedule/RenderScenePipeline.hpp>
#include <Engine.Input.Schedule/InputPipeline.hpp>

#include "ActorHousekeepingStage.hpp"
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
	const auto actorHousekeeping = register_.registerStage(
		make_uptr<ActorHousekeepingStage>(ActorHousekeeping, this)
	);

	/**/

	_orderedStages.emplace_back(actorTick);
	_orderedStages.emplace_back(actorHousekeeping);
}

void ActorPipeline::declareDependencies(
	cref<StageRegister> register_,
	ref<CompactSet<StageDependency>> collection_
) {
	const auto* const beginTick = register_.getStage(core::schedule::CorePipeline::TickBegin);
	const auto* const inputTick = register_.getStage(input::schedule::InputPipeline::InputTick);

	// TODO: Guarantee that tick end stage is latest within dependent chain
	// Attention: Hotfix
	const auto* const renderScene = register_.getStage(gfx::schedule::RenderScenePipeline::RenderTick);

	/**/

	const auto* const actorTick = _orderedStages.front();
	const auto* const actorHousekeeping = _orderedStages.back();

	/**/

	collection_.insert(
		StageDependency {
			{ beginTick, inputTick },
			this,
			actorTick
		}
	);
	collection_.insert(
		StageDependency {
			{ renderScene },
			this,
			actorHousekeeping
		}
	);
}

void ActorPipeline::dismount(ref<StageRegister> register_) {

	register_.removeStage(ActorHousekeeping);
	register_.removeStage(ActorTick);

	/**/

	_orderedStages.clear();
}

bool ActorPipeline::isSkippable() const noexcept {
	return false;
}
