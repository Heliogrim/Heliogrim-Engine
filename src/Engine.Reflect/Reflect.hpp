#pragma once

#include "HeliogrimReflect.hpp"

namespace hg {
    class Reflect {
    public:
        template <typename ActorType_>
        struct SubstitudeActorClass {
            FORCE_INLINE static ptr<HeliogrimClass> Known() noexcept {
                return HeliogrimClass::of<ActorType_>();
            }

            FORCE_INLINE static ptr<HeliogrimClass> Unknown() noexcept {
                return HeliogrimClass::of<ActorType_>();
            }
        };
    };
}
