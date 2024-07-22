#include "Stage.hpp"

using namespace hg::engine::accel;
using namespace hg;

void Stage::enumerateStageInputs(ref<Vector<StageInput>> inputs_) const noexcept {
	const u8 mask = static_cast<u8>(_flagBits);
	_intermediate->enumerateInboundSymbols(reinterpret_cast<ref<Vector<nmpt<const lang::Symbol>>>>(inputs_), mask);
}

void Stage::enumerateStageOutputs(ref<Vector<StageOutput>> outputs_) const noexcept {
	const u8 mask = static_cast<u8>(_flagBits);
	_intermediate->enumerateOutboundSymbols(reinterpret_cast<ref<Vector<nmpt<const lang::Symbol>>>>(outputs_), mask);
}
