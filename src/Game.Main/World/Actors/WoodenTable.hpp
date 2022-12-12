#pragma once
#include <Ember/Actor.hpp>

namespace ember::game::actors {
    class WoodenTable :
        public Actor {
    public:
        using this_type = WoodenTable;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 25.11.2021
         *
         * @see Actor::Actor
         */
        WoodenTable();

        WoodenTable(cref<ActorInitializer> initializer_);
    };
}
