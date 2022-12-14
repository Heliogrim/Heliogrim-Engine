#pragma once
#include <Ember/Actor.hpp>

namespace ember::game::actors {
    class WoodenTable :
        public Actor {
    public:
        using this_type = WoodenTable;

    public:
        WoodenTable(cref<ActorInitializer> initializer_);
    };
}
