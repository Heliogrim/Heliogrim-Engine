#pragma once
#include <atomic>
#include <Engine.Common/Hash/Murmur3.hpp>
#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.Common/Guid.hpp>
#include <chrono>

namespace hg {
    /**
     * An guid actor identifier.
     *
     * @author Julius
     * @date 31.08.2020
     */
    struct actor_guid : Guid {
        using Guid::Guid;
    };

    /**
     * Identifier for the invalid actor
     */
    inline constexpr static actor_guid invalid_actor_guid {};

    /**
     * Generates an actor unique identifier
     *
     * @author Julius
     * @date 23.10.2020
     *
     * @returns The actor unique identifier.
     */
    [[maybe_unused]] static actor_guid generate_actor_guid() {
        static std::atomic_uint_fast32_t ai { 1 };
        const auto pre = ai.fetch_add(1, std::memory_order_relaxed);
        const auto post = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        return { pre, 0, 0, post };
    }
}

namespace std {
    /**
     * Specific hash of actor_guid
     *
     * @author Julius
     * @date 29.10.2020
     */
    template <>
    struct hash<hg::actor_guid> {
        std::size_t operator()(const hg::actor_guid& value_) const noexcept {
            /**
             * !important! don't use identity
             */
            hg::u64 dst[2];
            hg::hash::murmur3_x64_128(
                &value_,
                sizeof(hg::actor_guid),
                0x9FB21C651E98DF25uL,
                &dst
            );
            return dst[0];
        }
    };
}
