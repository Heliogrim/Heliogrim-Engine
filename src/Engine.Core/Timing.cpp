#include "Timing.hpp"

using namespace hg::engine::core;
using namespace hg;

Opt<ref<Timing>> Timing::_cached = None;

Timing::Timing() noexcept :
	_lastTimePoint(std::chrono::high_resolution_clock::now()),
	_rawDeltaTime(0.F) {
	_cached = Some(*this);
}

Timing::~Timing() noexcept = default;

void Timing::updateTimings() {
	const auto nextTimePoint = std::chrono::high_resolution_clock::now();
	const auto diff = nextTimePoint - _lastTimePoint;
	_lastTimePoint = nextTimePoint;

	// 1s -> 1000ms
	// 1ms -> 1000us | 1000ms -> 1000'000us
	// 1us -> 1000ns | 1000us -> 1000'000ns | 1000'000us -> 1000'000'000ns
	_rawDeltaTime = static_cast<f32>(std::chrono::duration_cast<std::chrono::microseconds>(diff).count()) / 1'000'000.F;
}

f32 Timing::getDeltaTime() noexcept {
	return Timing::_cached.has_value() ? Timing::_cached->_rawDeltaTime : 0.F;
}
