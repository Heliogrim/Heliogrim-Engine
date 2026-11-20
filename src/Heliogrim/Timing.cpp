#include "Timing.hpp"

import Engine.Core;

using namespace hg::engine;
using namespace hg;

f32 hg::GetDeltaTime() noexcept {
	return core::Timing::getDeltaTime();
}
