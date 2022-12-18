#pragma once

#include "DependencyKey.hpp"

namespace ember::engine::core {
    /**/
    inline constexpr static DependencyKey PlatformDepKey { "::ember::engine::Platform"sv };
    inline constexpr static DependencyKey ResourcesDepKey { "::ember::engine::ResourceManager"sv };
    inline constexpr static DependencyKey SchedulerDepKey { "::ember::engine::Scheduler"sv };
    /**/
    inline constexpr static DependencyKey AssetsDepKey { "::ember::engine::Assets"sv };
    inline constexpr static DependencyKey AudioDepKey { "::ember::engine::Audio"sv };
    inline constexpr static DependencyKey GraphicsDepKey { "::ember::engine::Graphics"sv };
    inline constexpr static DependencyKey InputDepKey { "::ember::engine::Input"sv };
    inline constexpr static DependencyKey NetworkDepKey { "::ember::engine::Network"sv };
    inline constexpr static DependencyKey PhysicsDepKey { "::ember::engine::Physics"sv };
}
