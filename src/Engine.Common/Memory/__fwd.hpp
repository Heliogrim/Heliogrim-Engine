#pragma once

#include "../__macro.hpp"

namespace hg {
    template <typename Ty_, typename AllocType_ = _STD allocator<Ty_>, bool Atomic_ = false>
    struct MemoryPointerStorage;

    template <
        typename Ty_,
        typename AllocType_ = _STD allocator<Ty_>,
        typename StorageType_ = MemoryPointerStorage<Ty_, AllocType_, false>>
    struct MemoryPointer;

    template <
        typename Ty_,
        typename AllocType_ = _STD allocator<Ty_>,
        typename StorageType_ = MemoryPointerStorage<Ty_, AllocType_, true>>
    using AtomicMemoryPointer = MemoryPointer<Ty_, AllocType_, StorageType_>;

    /**/

    template <
        typename Ty_,
        typename StorageType_ = MemoryPointerStorage<Ty_, _STD allocator<Ty_>, false>>
    struct NonOwningMemoryPointer;

    template <
        typename Ty_,
        typename StorageType_ = MemoryPointerStorage<Ty_, _STD allocator<Ty_>, true>>
    using AtomicNonOwningMemoryPointer = NonOwningMemoryPointer<Ty_, StorageType_>;

    template <typename Ty_>
    using ConstMemoryPointer = NonOwningMemoryPointer<Ty_>;

    template <typename Ty_>
    using ConstAtomicMemoryPointer = AtomicNonOwningMemoryPointer<Ty_>;
}
