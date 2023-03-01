#pragma once

namespace hg::engine::resource {
    enum class LinkerStoreMode {
        /**
         * When linker uses `eKeepAlive` mechanism, the lifetime of each individual linked archive will be expanded until
         *  the underlying package gets finalized and therefore linearly written.
         */
        eKeepAlive,
        /**
         * When linker uses `eFlushAndFrag` mechanism, the sub-sequential stored content will be written and flush directly
         *  to the underlying package while the linker does the bookkeeping of the unordered writes.
         *  While finalizing the package, the content will be rewritten (defragmented) and therefore linearly overwritten.
         */
        eFlushAndDefrag
    };
}
