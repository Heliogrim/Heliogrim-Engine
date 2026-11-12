#include "InputPipeline.hpp"

#include <Engine.Common/Make.hpp>

#include "InputTickStage.hpp"

using namespace hg::engine::input::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

InputPipeline::InputPipeline() :
	StagePipeline(identifier_type::from("::Input::InputPipeline")) {}

InputPipeline::~InputPipeline() = default;

void InputPipeline::mount(ref<StageRegister> register_) {

	(void)register_.registerStage(
		make_uptr<InputTickStage>(InputTick, this)
	);
}

void InputPipeline::dismount(ref<StageRegister> register_) {

	register_.removeStage(InputTick);
}
