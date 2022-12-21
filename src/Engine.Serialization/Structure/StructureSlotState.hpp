#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../Archive/__fwd.hpp"
#include "StructureSlotStateFlags.hpp"

namespace ember::engine::serialization {
    class StructureSlotState final {
    public:
        using this_type = StructureSlotState;

    public:
        StructureSlotState(const non_owning_rptr<Archive> archive_);

        ~StructureSlotState();

    private:
        StructureSlotStateFlags _flags;

    public:
        [[nodiscard]] cref<StructureSlotStateFlags> getStateFlags() const noexcept;

        [[nodiscard]] ref<StructureSlotStateFlags> stateFlags() noexcept;

    private:
        non_owning_rptr<Archive> _archive;

    public:
        [[nodiscard]] const non_owning_rptr<Archive> getArchive() const noexcept;
    };
}
