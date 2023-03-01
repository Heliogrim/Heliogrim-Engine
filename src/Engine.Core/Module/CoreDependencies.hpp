#pragma once

#include "DependencyKey.hpp"

namespace hg::engine::core {
    /**/
    inline constexpr static DependencyKey PlatformDepKey { "::hg::engine::Platform"sv };
    inline constexpr static DependencyKey ResourcesDepKey { "::hg::engine::ResourceManager"sv };
    inline constexpr static DependencyKey SchedulerDepKey { "::hg::engine::Scheduler"sv };
    /**/
    inline constexpr static DependencyKey AssetsDepKey { "::hg::engine::Assets"sv };
    inline constexpr static DependencyKey AudioDepKey { "::hg::engine::Audio"sv };
    inline constexpr static DependencyKey GraphicsDepKey { "::hg::engine::Graphics"sv };
    inline constexpr static DependencyKey InputDepKey { "::hg::engine::Input"sv };
    inline constexpr static DependencyKey NetworkDepKey { "::hg::engine::Network"sv };
    inline constexpr static DependencyKey PhysicsDepKey { "::hg::engine::Physics"sv };
}
