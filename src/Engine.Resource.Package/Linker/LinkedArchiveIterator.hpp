#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "PackageLinker.hpp"
#include "__fwd.hpp"
#include <Engine.Serialization/Archive/__fwd.hpp>

namespace ember::engine::resource {
    template <bool Const_>
    class LinkedArchiveIterator {
    public:
        friend class PackageLinker;

    public:
        using this_type = LinkedArchiveIterator<Const_>;

        using value_type = _STD conditional_t<Const_, const LinkedArchive, LinkedArchive>;
        using reference_type = _STD conditional_t<Const_, cref<LinkedArchive>, ref<LinkedArchive>>;
        using const_reference_type = cref<value_type>;
        using pointer_type = ptr<value_type>;

        using archive_type = _STD conditional_t<
            Const_,
            serialization::SourceReadonlyArchive,
            serialization::SourceBaseArchive
        >;
        using header_type = _STD conditional_t<Const_, const ArchiveHeader, ArchiveHeader>;
        using linker_type = _STD conditional_t<Const_, const PackageLinker, PackageLinker>;

        // Iterator Traits
        using difference_type = _STD ptrdiff_t;
        using size_type = _STD size_t;
        using pointer = pointer_type;
        using reference = reference_type;

        #ifdef __cpp_lib_concepts
        using iterator_concept = _STD random_access_iterator_tag;
        #endif

        using iterator_category = _STD random_access_iterator_tag;

        //
        constexpr static inline bool IsConst = Const_;

    public:
        LinkedArchiveIterator() = delete;

        LinkedArchiveIterator(const non_owning_rptr<linker_type> linker_) :
            _linker(linker_),
            _cursor(linker_->_links.data()) {}

        LinkedArchiveIterator(const non_owning_rptr<linker_type> linker_, const size_type pos_) :
            _linker(linker_),
            _cursor(linker_->_links.data() + pos_) {}

        LinkedArchiveIterator(cref<this_type> other_) noexcept :
            _linker(other_._linker),
            _cursor(other_._cursor) {}

        LinkedArchiveIterator(mref<this_type> other_) noexcept :
            _linker(_STD exchange(other_._linker, nullptr)),
            _cursor(_STD exchange(other_._cursor, nullptr)) {}

        ~LinkedArchiveIterator() noexcept = default;

    private:
        non_owning_rptr<linker_type> _linker;
        pointer_type _cursor;

    public:
        [[nodiscard]] uptr<archive_type> archive() const noexcept {
            return (*_linker)[*this];
        }

        [[nodiscard]] header_type header() const noexcept {
            return _cursor->header;
        }

    public:
        [[nodiscard]] const_reference_type operator*() const noexcept {
            return *_cursor;
        }

        [[nodiscard]] reference_type operator*() noexcept {
            return *_cursor;
        }

        [[nodiscard]] const pointer_type operator->() const noexcept {
            return _cursor;
        }

    public:
        [[nodiscard]] const_reference_type operator[](const size_type offset_) const noexcept {
            return _cursor[offset_];
        }

        [[nodiscard]] reference_type operator[](const size_type offset_) noexcept {
            return _cursor[offset_];
        }

    public:
        ref<this_type> operator++() {
            return ++_cursor, *this;
        }

        [[nodiscard]] this_type operator++(int) {
            auto tmp { *this };
            ++_cursor;
            return tmp;
        }

        ref<this_type> operator--() {
            return --_cursor, *this;
        }

        [[nodiscard]] this_type operator--(int) {
            auto tmp { *this };
            --_cursor;
            return tmp;
        }

    public:
        ref<this_type> operator+=(const size_type distance_) {
            return _cursor += distance_, *this;
        }

        ref<this_type> operator-=(const size_type distance_) {
            return _cursor -= distance_, *this;
        }

    public:
        [[nodiscard]] this_type operator+(const size_type distance_) const {
            auto tmp { *this };
            return tmp += distance_, tmp;
        }

        [[nodiscard]] this_type operator-(const size_type distance_) const {
            auto tmp { *this };
            return tmp -= distance_, tmp;
        }

    public:
        [[nodiscard]] bool operator==(cref<this_type> other_) const noexcept {
            return _cursor == other_._cursor;
        }

        [[nodiscard]] bool operator!=(cref<this_type> other_) const noexcept {
            return _cursor != other_._cursor;
        }

        [[nodiscard]] bool operator<(cref<this_type> other_) const noexcept {
            return _cursor < other_._cursor;
        }

        [[nodiscard]] bool operator>(cref<this_type> other_) const noexcept {
            return _cursor > other_._cursor;
        }

        [[nodiscard]] bool operator<=(cref<this_type> other_) const noexcept {
            return _cursor <= other_._cursor;
        }

        [[nodiscard]] bool operator>=(cref<this_type> other_) const noexcept {
            return _cursor >= other_._cursor;
        }

        [[nodiscard]] _STD strong_ordering operator<=>(cref<this_type> other_) const noexcept {
            return _STD _Unfancy(_cursor) <=> _STD _Unfancy(other_._cursor);
        }
    };

    template <bool ConstLeft_, bool ConstRight_>
    [[nodiscard]] bool operator==(
        cref<LinkedArchiveIterator<ConstLeft_>> left_,
        cref<LinkedArchiveIterator<ConstRight_>> right_
    ) noexcept {
        using const_iterator = LinkedArchiveIterator<true>;
        return reinterpret_cast<cref<const_iterator>>(left_) == reinterpret_cast<cref<const_iterator>>(right_);
    }

    template <bool ConstLeft_, bool ConstRight_>
    [[nodiscard]] bool operator!=(
        cref<LinkedArchiveIterator<ConstLeft_>> left_,
        cref<LinkedArchiveIterator<ConstRight_>> right_
    ) noexcept {
        using const_iterator = LinkedArchiveIterator<true>;
        return reinterpret_cast<cref<const_iterator>>(left_) != reinterpret_cast<cref<const_iterator>>(right_);
    }

    template <bool ConstLeft_, bool ConstRight_>
    [[nodiscard]] _STD strong_ordering operator<=>(
        cref<LinkedArchiveIterator<ConstLeft_>> left_,
        cref<LinkedArchiveIterator<ConstRight_>> right_
    ) noexcept {
        using const_iterator = LinkedArchiveIterator<true>;
        return reinterpret_cast<cref<const_iterator>>(left_) <=> reinterpret_cast<cref<const_iterator>>(right_);
    }
}
