#pragma once

#include "__fwd.hpp"
#include "../Structure/__fwd.hpp"
#include "../Structure/RootScopedSlot.hpp"

namespace hg::engine::serialization {
    class StructuredArchive {
    public:
        using this_type = StructuredArchive;

    public:
        StructuredArchive(const ptr<Archive> archive_);

        StructuredArchive(cref<this_type>) = delete;

        StructuredArchive(mref<this_type>) noexcept = delete;

        ~StructuredArchive() = default;

    private:
        const ptr<Archive> _archive;

    public:
        [[nodiscard]] const ptr<Archive> getArchive() const noexcept;

    public:
        [[nodiscard]] RootScopedSlot getRootSlot() const;

        [[nodiscard]] RootScopedSlot insertRootSlot();
    };
}
