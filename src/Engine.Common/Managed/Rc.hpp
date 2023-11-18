#pragma once
#include "../Concurrent/SharedMemoryReference.hpp"

namespace hg {
    /**
     * Reference Counted < Type_ >
     */
    template <typename Type_>
    using Rc = smr<Type_>;

    /**
     * Intrusive Reference Counted < Type_ >
     */
    template <typename Type_>
    using IRc = smr<Type_>;

    /**
     * Shared Reference Counted < Type_ >
     */
    template <typename Type_>
    using SRc = smr<Type_>;

    /**
     * Shared Intrusive Reference Counted < Type_ >
     */
    template <typename Type_>
    using SIRc = smr<Type_>;
}
