#pragma once

#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Common/Hash/Fnv-1a.hpp>

namespace hg::engine::gfx::acc {
    using Token = ::hg::AssocKey<string, hasher::fnv1a<string>>;
}
