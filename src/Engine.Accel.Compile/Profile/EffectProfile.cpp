#include "EffectProfile.hpp"

using namespace hg::engine::accel;
using namespace hg;

cref<ProfileUsagePattern> EffectProfile::getUsagePattern() const noexcept {
    return _usagePattern;
}
