#pragma once
#include <Heliogrim/Actor.hpp>

namespace hg::game::actors {
    class EditorCamera :
        public InheritMeta<EditorCamera, Actor> {
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
        EditorCamera() = default;
    };
}
