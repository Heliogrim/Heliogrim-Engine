#pragma once

#include "__fwd.hpp"
#include "../Structure/__fwd.hpp"
#include "../Structure/RootStructureSlot.hpp"

namespace ember::engine::serialization {
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
        [[nodiscard]] const RootStructureSlot getRootSlot() const;

        [[nodiscard]] RootStructureSlot getRootSlot();

    public:
        template <typename Type_>
        ref<this_type> operator<<(
            _In_ cref<_STD pair<StructurePath, const ptr<const Type_>>> entry_
        ) {
            return *this;
        }

        ref<this_type> operator>>(
            _Inout_ ref<_STD pair<StructurePath, const ptr<const ScopedStructureSlotBase>>> entry_
        ) {
            return *this;
        }
    };
}
