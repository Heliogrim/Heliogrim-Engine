#pragma once
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Accel.Lang/Intermediate.hpp>

#include "ProfileDefinition.hpp"
#include "ProfileUsagePattern.hpp"

namespace hg::engine::accel {
    class EffectProfile {
    public:
        string _name;

    public:
        Vector<ProfileDefinition> _definitions;
        Vector<lang::Intermediate> _modules;

    private:
    public:
        ProfileUsagePattern _usagePattern;

    public:
        [[nodiscard]] cref<ProfileUsagePattern> getUsagePattern() const noexcept;
    };
}
