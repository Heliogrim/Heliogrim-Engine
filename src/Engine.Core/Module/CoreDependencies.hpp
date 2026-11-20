#pragma once

#include "DependencyKey.hpp"

namespace hg::engine::core {
	/**/
	inline constexpr DependencyKey ActorDepKey { "::hg::engine::ActorModule"sv };
	inline constexpr DependencyKey AssetsDepKey { "::hg::engine::Assets"sv };
	inline constexpr DependencyKey AudioDepKey { "::hg::engine::Audio"sv };
	inline constexpr DependencyKey GraphicsDepKey { "::hg::engine::Graphics"sv };
	inline constexpr DependencyKey InputDepKey { "::hg::engine::Input"sv };
	inline constexpr DependencyKey NetworkDepKey { "::hg::engine::Network"sv };
	inline constexpr DependencyKey PhysicsDepKey { "::hg::engine::Physics"sv };
}
