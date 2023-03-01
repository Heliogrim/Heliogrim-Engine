#pragma once
#include <Heliogrim/Actor.hpp>

namespace hg::game::actors {
    class WoodenTable :
        public Actor {
    public:
        using this_type = WoodenTable;

    public:
        WoodenTable(cref<ActorInitializer> initializer_);
    };
}
