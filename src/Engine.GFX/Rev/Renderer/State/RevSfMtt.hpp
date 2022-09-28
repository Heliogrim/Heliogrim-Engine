#pragma once

#include <atomic>
#include <Engine.Common/Collection/RobinMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX/Resource/MaterialResource.hpp>

namespace ember::engine::gfx::render {

    struct RevSfMtt {
        _STD atomic_uint_fast16_t mti;

        RobinMap<ptr<gfx::MaterialResource>, u32> forward;
        Vector<ptr<gfx::MaterialResource>> backward;

    public:
        [[nodiscard]] u32 insert(const ptr<gfx::MaterialResource> material_) {

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
