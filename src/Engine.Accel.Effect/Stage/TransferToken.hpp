#pragma once

#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Hash/Fnv-1a.hpp>

namespace hg::engine::accel {
    using TransferToken = AssocKey<string, hasher::fnv1a<string>>;
}
