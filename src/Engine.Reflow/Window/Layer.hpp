#pragma once

#include <utility>
#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"
#include "../Widget/Layer/Host.hpp"

namespace hg::engine::reflow {
	using Layer = std::pair<sptr<Window>, wptr<Host>>;
}
