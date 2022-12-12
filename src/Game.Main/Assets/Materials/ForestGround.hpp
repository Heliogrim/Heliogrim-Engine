#pragma once
#include "./Inc.hpp"

namespace clockwork::game::asset {
    class ForestGround :
        public Material {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 18.06.2020
         */
        ForestGround() :
            Material() {
            _id = Url("material::ForestGround");
        }

    private:
    };
}
