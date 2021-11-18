#include "DynamicBufferPages.hpp"

using namespace ember::engine::scheduler;
using namespace ember;

DynamicBufferPages::DynamicBufferPages(const u64 size_, const ptr<std::atomic_flag> tidySignal_) :
    _tidySignal(tidySignal_),
    _pages(nullptr),
    _size(size_) {
    allocate();
}

DynamicBufferPages::~DynamicBufferPages() {
    tidy();
}

void DynamicBufferPages::tidy(const bool cascade_) {
    /**
     *
     */
    if (_pages != nullptr) {

        /**
         * Check whether we have to free indirected page value types
         */
        if (cascade_) {
            for (auto i = 0; i < _size; ++i) {
                auto value = _pages[i];
                delete value;
            }
        }

        /**
         * Deallocate current resources and nullify
         */
        delete[] _pages;
        _pages = nullptr;
        _size = 0;
    }

    /**
     *
     */
    if (_tidySignal) {
        _tidySignal->clear(_STD memory_order_release);
        _tidySignal = nullptr;
    }
}

void DynamicBufferPages::release(ptr<DynamicBufferPages> self_) {
    delete self_;
}

void DynamicBufferPages::allocate() {

    /**
     * Create new pages
     */
    auto* pages = new page_type[_size];

    /**
     * Pointer cleanup
     */
    for (auto i = 0; i < _size; ++i) {
        pages[i] = nullptr;
    }

    /**
     * Store
     */
    _pages = pages;
}

DynamicBufferPages::page_type DynamicBufferPages::page(const u64 idx_) const noexcept {
    DEBUG_ASSERT(idx_ < _size, "Index out of bound.")
    return _pages[idx_];
}

ref<DynamicBufferPages::page_type> DynamicBufferPages::page(const u64 idx_) noexcept {
    DEBUG_ASSERT(idx_ < _size, "Index out of bound.")
    return _pages[idx_];
}

u64 DynamicBufferPages::size() const noexcept {
    return _size;
}
