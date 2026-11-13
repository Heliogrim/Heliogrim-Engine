#include "InputPipeline.hpp"

#include <ranges>
#include <Engine.Common/Make.hpp>

#include "InputTickStage.hpp"

using namespace hg::engine::input::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

InputPipeline::InputPipeline() :
	StagePipeline(identifier_type::from("::Input::InputPipeline")) {}

InputPipeline::~InputPipeline() = default;

void InputPipeline::mount(ref<StageRegister> register_) {

	_stages = {
		register_.registerStage(
			make_uptr<InputTickStage>(InputTick, this)
		)
	};
}

void InputPipeline::dismount(ref<StageRegister> register_) {

	for (auto& stage : std::ranges::views::reverse(_stages)) {
		if (stage != None) {
			register_.removeStage(std::exchange(stage, None).value());
		}
	}
}
