#pragma once

#include <Engine.ACS/Traits.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg {
    /**
     * Forward Declaration
     */
    class Actor;

    struct CachedActorPointer {
        /**
         *
         */
        actor_guid guid = invalid_actor_guid;

        /**
         *
         */
        ptr<Actor> cached = nullptr;

    public:
        [[nodiscard]] ref<Actor> operator*() const noexcept {
            return *cached;
        }

        [[nodiscard]] ptr<Actor> operator->() const noexcept {
            return cached;
        }
    };
}
