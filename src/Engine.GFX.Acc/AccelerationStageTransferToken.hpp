#pragma once

#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Hash/Fnv-1a.hpp>

namespace hg::engine::gfx::acc {
    using AccelerationStageTransferToken = AssocKey<string/* TODO: hash::fnv1a */>;
}
