#pragma once

#include <atomic>
#include <Engine.Common/Collection/RobinMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>

namespace hg::engine::gfx::glow::render {
    struct RevSfMtt {
        _STD atomic_uint_fast16_t mti;

        RobinMap<ptr<MaterialResource>, u32> forward;
        Vector<ptr<MaterialResource>> backward;

    public:
        [[nodiscard]] u32 insert(const ptr<MaterialResource> material_) {

            const auto exists { forward.find(material_) };
            if (exists != forward.end()) {
                return exists->second;
            }

            auto next { mti++ };
            auto fr { forward.insert(_STD make_pair(material_, next)) };
            assert(fr.second);

            if (backward.size() <= next) {
                backward.resize(next + 1ui16);
            }
            backward[next] = material_;

            return next;
        }
    };
}
