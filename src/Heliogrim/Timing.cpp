#include "Timing.hpp"

#include <Engine.Core/Timing.hpp>

using namespace hg::engine;
using namespace hg;

f32 hg::GetDeltaTime() noexcept {
	return core::Timing::getDeltaTime();
}
