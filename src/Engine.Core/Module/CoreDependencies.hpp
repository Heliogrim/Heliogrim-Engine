#pragma once

#include "DependencyKey.hpp"

namespace hg::engine::core {
	/**/
	constexpr static DependencyKey ActorDepKey { "::hg::engine::ActorModule"sv };
	constexpr static DependencyKey AssetsDepKey { "::hg::engine::Assets"sv };
	constexpr static DependencyKey AudioDepKey { "::hg::engine::Audio"sv };
	constexpr static DependencyKey GraphicsDepKey { "::hg::engine::Graphics"sv };
	constexpr static DependencyKey InputDepKey { "::hg::engine::Input"sv };
	constexpr static DependencyKey NetworkDepKey { "::hg::engine::Network"sv };
	constexpr static DependencyKey PhysicsDepKey { "::hg::engine::Physics"sv };
}
