#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::scene {
    template <class ElementType_, class Traits_>
    class SceneElementStorage {
    public:
        [[nodiscard]] ptr<ptr<ElementType_>> acquire() {
            return new ptr<ElementType_> [Traits_::max_elements_per_leaf];
        }

        void release(ptr<ptr<ElementType_>> res_) {
            delete[] res_;
        }
    };
}
