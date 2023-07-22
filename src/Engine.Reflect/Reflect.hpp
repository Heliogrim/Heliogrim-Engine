#pragma once
#include "Meta/TypedMetaClass.hpp"

namespace hg {
    class Reflect {
    public:
        template <typename ActorType_>
        struct SubstitudeActorClass {
            FORCE_INLINE static ptr<const MetaClass> Known() noexcept {
                return TypedMetaClass<ActorType_>::get();
            }

            FORCE_INLINE static ptr<const MetaClass> Unknown() noexcept {
                return TypedMetaClass<ActorType_>::get();
            }
        };
    };
}
