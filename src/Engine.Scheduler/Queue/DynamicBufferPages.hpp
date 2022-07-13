#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "AtomicCtrlBlockPage.hpp"
#include "SharedBufferPool.hpp"

namespace ember::engine::scheduler {

    class DynamicBufferPages {
    public:
        using size_type = uint_fast16_t;
        inline static constexpr size_type page_size = 16ui16;

        using page_value_type = AtomicCtrlBlockPage<SharedBufferPool::aligned_buffer, page_size>;
        using page_type = ptr<page_value_type>;

    public:
        DynamicBufferPages(_In_ const u64 size_, _In_ const ptr<_STD atomic_flag> tidySignal_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 17.11.2021
         *
         * @details This will invoke tidy with falsified cascade parameter
         */
        ~DynamicBufferPages();

    public:
        /**
         * Cleanup this dynamic pages by deallocating current pages and if active, cascaded delete operation
         *
         * @author Julius
         * @date 17.11.2021
         */
        void tidy(_In_ const bool cascade_ = false);

    private:
        ptr<_STD atomic_flag> _tidySignal;

    public:
        /**
         * Helper function to softly release given object
         *
         * @author Julius
         * @date 17.11.2021
         *
         * @param self_ The instance to release
         */
        static void release(_Inout_ ptr<DynamicBufferPages> self_);

    private:
        ptr<page_type> _pages;
        u64 _size;

    private:
        /**
         * Will acquire a memory sequence to at least store the requested amount of indirection pointers
         *
         * @author Julius
         * @date 17.11.2021
         */
        void allocate();

    public:
        /**
         * Get a pointer to the indexed page
         *
         * @author Julius
         * @date 17.11.2021
         *
         * @param idx_ The index of the requested page.
         *
         * @returns A pointer of the requested page
         */
        [[nodiscard]] page_type page(_In_ const u64 idx_) const noexcept;

        /**
         * Get a reference to the pointer of the indexed page
         *
         * @author Julius
         * @date 17.11.2021
         *
         * @param idx_ The index of the requested page.
         *
         * @returns A reference to the pointer stored page.
         */
        [[nodiscard]] ref<page_type> page(_In_ const u64 idx_) noexcept;

        /**
         * Get the current amount of pages stored
         *
         * @author Julius
         * @date 17.11.2021
         *
         * @returns The amount of stored pages, otherwise 0.
         */
        [[nodiscard]] u64 size() const noexcept;
    };
}
