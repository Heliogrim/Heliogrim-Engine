#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::reflow::theming {
	class Theming;
}

namespace hg::engine::reflow::uikit {
	extern Opt<ref<const theming::Theming>> getRuntimeTheming() noexcept;
}
