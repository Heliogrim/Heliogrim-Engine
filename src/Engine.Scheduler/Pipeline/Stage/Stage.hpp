#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Types.hpp>

#include "StageIdentifier.hpp"

namespace hg::engine::scheduler {
    class Stage final {
    public:
        using this_type = Stage;

    public:
        Stage(cref<StageIdentifier> identifier_, const s8 slot_) noexcept;

        Stage(cref<this_type>) = delete;

        Stage(mref<this_type>) noexcept = delete;

        ~Stage() noexcept = default;

    private:
        u16 _refCount;

    public:
        void incRef() noexcept;

        [[nodiscard]] bool decRef() noexcept;

    private:
        const StageIdentifier _identifier;
        const s8 _slot;

    public:
        [[nodiscard]] cref<StageIdentifier> getIdentifier() const noexcept;

        [[nodiscard]] s8 getSlot() const noexcept;
    };
}
