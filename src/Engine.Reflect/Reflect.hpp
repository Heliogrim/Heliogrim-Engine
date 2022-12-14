#pragma once

#include <Ember/Actor.hpp>
#include "EmberReflect.hpp"

namespace ember {
    class Reflect {
    public:
        template <typename ActorType_>
        struct SubstitudeActorClass {
            FORCE_INLINE static ptr<EmberClass> Known() noexcept {
                return EmberClass::of<ActorType_>();
            }

            FORCE_INLINE static ptr<EmberClass> Unknown() noexcept {
                return EmberClass::of<ActorType_>();
            }
        };
    };
}
