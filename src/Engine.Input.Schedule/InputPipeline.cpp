#include "InputPipeline.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>

#include "InputTickStage.hpp"

using namespace hg::engine::input::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

InputPipeline::InputPipeline() :
	StagePipeline(identifier_type::from("::Input::InputPipeline")) {}

InputPipeline::~InputPipeline() = default;

void InputPipeline::mount(ref<StageRegister> register_) {

	const auto inputTick = register_.registerStage(
		make_uptr<InputTickStage>(InputTick, this)
	);

	/**/

	_orderedStages.emplace_back(inputTick.address());
}

void InputPipeline::declareDependencies(
	cref<StageRegister> register_,
	ref<CompactSet<StageDependency>> collection_
) {
	const auto beginTick = register_.getStage(core::schedule::CorePipeline::TickBegin);
	::hg::assertrt(beginTick != None);

	/**/

	const auto* const inputTick = _orderedStages.front();

	/**/

	collection_.insert(
		StageDependency {
			{ beginTick.address() },
			this,
			inputTick
		}
	);
}

void InputPipeline::dismount(ref<StageRegister> register_) {

	register_.removeStage(InputTick);

	/**/

	_orderedStages.clear();
}

bool InputPipeline::isSkippable() const noexcept {
	return false;
}
