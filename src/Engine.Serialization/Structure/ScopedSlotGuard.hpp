#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"

namespace ember::engine::serialization {
    /**
     * RAII Guard for `complete` mutating slot operations
     */
    class ScopedSlotGuard final {
    public:
        using this_type = ScopedSlotGuard;

    public:
        explicit ScopedSlotGuard(const non_owning_rptr<const ScopedStructureSlotBase> slot_, const bool mutating_);

        ScopedSlotGuard(cref<this_type>) = delete;

        ScopedSlotGuard(mref<this_type>) noexcept = delete;

        ~ScopedSlotGuard();

    private:
        const bool _mutating;
        const non_owning_rptr<const ScopedStructureSlotBase> _slot;
    };
}
