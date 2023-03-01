#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::reflow {
    template <typename EntryType_>
    struct StyleSheetEntry {
        using this_type = StyleSheetEntry<EntryType_>;

        constexpr StyleSheetEntry(const bool set_, const EntryType_ attr_) :
            set(set_),
            attr(attr_) {}

        constexpr StyleSheetEntry(const EntryType_ attr_) :
            set(false),
            attr(attr_) {}

        StyleSheetEntry(cref<this_type> other_) noexcept(_STD is_nothrow_copy_constructible_v<EntryType_>) :
            set(other_.set),
            attr(other_.attr) {}

        StyleSheetEntry(mref<this_type> other_) noexcept(_STD is_nothrow_move_constructible_v<EntryType_>) :
            set(other_.set),
            attr(_STD move(other_.attr)) {}

        ~StyleSheetEntry() = default;

    public:
        ref<this_type> operator=(cref<this_type> other_) noexcept (_STD is_nothrow_copy_assignable_v<EntryType_>) {

            if (this != _STD addressof(other_)) {
                set = other_.set;
                attr = other_.attr;
            }

            return *this;
        }

        ref<this_type> operator=(mref<this_type> other_) noexcept (_STD is_nothrow_move_assignable_v<EntryType_>) {

            if (this != _STD addressof(other_)) {
                set = other_.set;
                attr = _STD move(other_.attr);
            }

            return *this;
        }

    public:
        bool set;
        EntryType_ attr;

    public:
        operator const EntryType_&() const noexcept {
            return attr;
        }

        operator EntryType_&() noexcept {
            return attr;
        }

        operator EntryType_() const noexcept {
            return attr;
        }

    public:
        const ptr<const EntryType_> operator->() const noexcept {
            return &attr;
        }

        ptr<EntryType_> operator->() noexcept {
            return &attr;
        }

    public:
        void unset() {
            set = false;
        }

        ref<this_type> operator=(cref<EntryType_> value_) noexcept(_STD is_nothrow_copy_assignable_v<EntryType_>) {
            set = true;
            attr = value_;

            return *this;
        }

        ref<this_type> operator=(mref<EntryType_> value_) noexcept(_STD is_nothrow_move_assignable_v<EntryType_>) {
            set = true;
            attr = _STD move(value_);

            return *this;
        }
    };
}
