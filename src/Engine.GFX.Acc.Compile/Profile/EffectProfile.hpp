#pragma once
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "ProfileDefinition.hpp"

namespace hg::engine::gfx::acc {
    class EffectProfile {
    public:
        string _name;

    public:
        Vector<ProfileDefinition> _definitions;
    };
}
