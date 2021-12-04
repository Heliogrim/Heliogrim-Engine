#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::scene {

    template <class Type_, class Traits_>
    class SceneElementStorage {
    public:
        [[nodiscard]] ptr<Type_> acquire() {
            return new Type_ [Traits_::max_elements_per_node];
        }

        void release(ptr<Type_> res_) {
            delete[] res_;
        }
    };
}
