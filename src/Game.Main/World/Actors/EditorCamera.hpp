#pragma once
#include <Ember/Actor.hpp>

namespace ember::game::actors {

    class EditorCamera :
        public Actor {
    public:
        using this_type = EditorCamera;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 25.11.2021
         *
         * @see Actor::Actor
         */
        EditorCamera();
    };

}
