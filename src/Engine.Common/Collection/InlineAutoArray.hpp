#pragma once

#include <cassert>
#include <memory>

#include "InlineArray.hpp"
#include "../Wrapper.hpp"

namespace hg {
    /*
    template <typename Type_, size_t Capacity_, typename Allocator_ = _STD allocator<Type_>>
    using InlineAutoArray = InlineArray<Type_, Capacity_>;
     */
    template <class Type_, size_t Capacity_, typename Allocator_>
    class InlineAutoArray;

    template <class Type_, typename Allocator_>
    class InlineAutoArray<Type_, 0, Allocator_> {
        static_assert(
            not _STD is_void_v<_STD void_t<Type_>>,
            "Did you mean to use AutoArray<Type_, ...> or Vector<Type_, ...> ?"
        );
    };

    template <class Type_, size_t Capacity_ = 1, typename Allocator_ = _STD allocator<Type_>>
    class InlineAutoArray {
    public:
        using this_type = InlineAutoArray<Type_, Capacity_, Allocator_>;

        using value_type = Type_;

        static constexpr auto element_align = alignof(value_type);
        static constexpr auto element_size = sizeof(value_type);
        static constexpr auto element_capacity = Capacity_;

        static constexpr auto inline_byte_size = element_size * Capacity_;

        using size_type = size_t;
        using difference_type = ptrdiff_t;

        // TODO: rebind_allocator
        using allocator_type = Allocator_;
        using allocator_traits = _STD allocator_traits<allocator_type>;

    public:
        constexpr InlineAutoArray() noexcept :
            _alloc(),
            _traits(),
            _inlineStorage(),
            _inlineEnd(inline_begin()),
            _externalStorage(nullptr),
            _externalEnd(nullptr) { }

        template <typename InitialType_ = Type_> requires
            (Capacity_ == 1uLL) &&
            _STD is_nothrow_move_constructible_v<Type_> &&
            _STD is_constructible_v<Type_, InitialType_>
        constexpr InlineAutoArray(InitialType_&& value_) noexcept :
            _alloc(),
            _traits(),
            _inlineStorage(),
            _inlineEnd(inline_begin()),
            _externalStorage(nullptr),
            _externalEnd(nullptr) {
            inline_emplace_back(_STD forward<InitialType_>(value_));
        }

        template <typename... InitialType_> requires
            (Capacity_ >= sizeof...(InitialType_)) &&
            _STD is_nothrow_move_constructible_v<Type_>
        constexpr InlineAutoArray(InitialType_&&... values_) noexcept :
            _alloc(),
            _traits(),
            _inlineStorage(),
            _inlineEnd(inline_begin()),
            _externalStorage(nullptr),
            _externalEnd(nullptr) {
            ((inline_emplace_back(_STD forward<InitialType_>(values_))), ...);
        }

        template <typename OtherType_ = this_type> requires
            _STD is_same_v<OtherType_, this_type> &&
            _STD is_nothrow_move_constructible_v<Type_>
        constexpr InlineAutoArray(mref<OtherType_> other_) noexcept :
            _alloc(_STD move(other_._alloc)),
            _traits(_STD move(other_._traits)),
            _inlineStorage(),
            _inlineEnd(inline_begin()),
            _externalStorage(_STD exchange(other_._externalStorage, nullptr)),
            _externalEnd(_STD exchange(other_._externalEnd, nullptr)) {
            for (auto* iIt = other_.inline_begin(); iIt < other_._inlineEnd; ++iIt) {
                inline_emplace_back(_STD move(*iIt));
            }
            _STD _Destroy_range(other_.inline_begin(), other_._inlineEnd, _alloc);
            other_._inlineEnd = other_.inline_begin();
        }

        constexpr InlineAutoArray(cref<this_type>) noexcept = delete;

        constexpr ~InlineAutoArray() noexcept {
            tidy();
        }

    public:
        ref<this_type> operator=(mref<this_type>) noexcept = delete;

        ref<this_type> operator=(cref<this_type>) noexcept = delete;

    private:
        allocator_type _alloc;
        allocator_traits _traits;

        _::byte _inlineStorage[inline_byte_size];
        ptr<Type_> _inlineEnd;

        ptr<Type_> _externalStorage;
        ptr<Type_> _externalEnd;

    public:
        template <bool Const_>
        class IteratorImpl {
        public:
            using this_type = InlineAutoArray<Type_, Capacity_, Allocator_>::IteratorImpl<Const_>;

            using iterator_category = _STD random_access_iterator_tag;
            using difference_type = InlineAutoArray<Type_, Capacity_, Allocator_>::difference_type;
            using value_type = _STD conditional_t<Const_, const Type_, Type_>;
            using pointer = _STD conditional_t<Const_, ptr<const Type_>, ptr<Type_>>;
            using reference = _STD conditional_t<Const_, cref<Type_>, ref<Type_>>;

            static constexpr bool is_const = Const_;

        public:
            constexpr IteratorImpl() :
                _inlineBase(nullptr),
                _externalBase(nullptr),
                _idx(0) {}

            constexpr IteratorImpl(
                const pointer inlineBase_,
                const pointer externalBase_,
                const difference_type idx_
            ) :
                _inlineBase(inlineBase_),
                _externalBase(externalBase_),
                _idx(idx_) {}

            constexpr IteratorImpl(const pointer inlineBase_, const pointer externalBase_) :
                IteratorImpl(inlineBase_, externalBase_, 0) {}

            constexpr IteratorImpl(mref<this_type> other_) noexcept :
                _inlineBase(other_._inlineBase),
                _externalBase(other_._externalBase),
                _idx(other_._idx) {}

            constexpr IteratorImpl(cref<this_type> other_) noexcept :
                _inlineBase(other_._inlineBase),
                _externalBase(other_._externalBase),
                _idx(other_._idx) {}

            constexpr ~IteratorImpl() noexcept = default;

        public:
            ref<this_type> operator=(mref<this_type> other_) noexcept = default;

            ref<this_type> operator=(cref<this_type> other_) noexcept = default;

        private:
            pointer _inlineBase;
            pointer _externalBase;
            difference_type _idx;

        private:
            [[nodiscard]] constexpr pointer retrieve() const noexcept {
                if (_idx < element_capacity) {
                    return &_inlineBase[_idx];
                }
                return &_externalBase[_idx - element_capacity];
            }

            [[nodiscard]] constexpr pointer retrieve() noexcept {
                if (_idx < element_capacity) {
                    return &_inlineBase[_idx];
                }
                return &_externalBase[_idx - element_capacity];
            }

        public:
            [[nodiscard]] constexpr pointer operator->() const noexcept {
                return retrieve();
            }

            [[nodiscard]] constexpr pointer operator->() noexcept {
                return retrieve();
            }

            [[nodiscard]] constexpr reference operator*() const noexcept {
                return *retrieve();
            }

            [[nodiscard]] constexpr reference operator*() noexcept {
                return *retrieve();
            }

        public:
            constexpr ref<this_type> operator+=(difference_type distance_) noexcept {
                _idx += distance_;
                return *this;
            }

            [[nodiscard]] constexpr this_type operator+(difference_type distance_) noexcept {
                auto tmp { *this };
                tmp += distance_;
                return tmp;
            }

            constexpr ref<this_type> operator++() noexcept {
                return ++_idx, *this;
            }

            [[nodiscard]] constexpr this_type operator++(int) noexcept {
                auto tmp { *this };
                ++(*this);
                return tmp;
            }

            constexpr ref<this_type> operator-=(difference_type distance_) noexcept {
                _idx -= distance_;
                return *this;
            }

            [[nodiscard]] constexpr this_type operator-(difference_type distance_) noexcept {
                auto tmp { *this };
                tmp -= distance_;
                return tmp;
            }

            constexpr ref<this_type> operator--() noexcept {
                return --_idx, *this;
            }

            [[nodiscard]] constexpr this_type operator--(int) noexcept {
                auto tmp { *this };
                --(*this);
                return tmp;
            }

        public:
            constexpr ref<this_type> operator[](difference_type distance_) noexcept {
                return *this += distance_;
            }

        public:
            [[nodiscard]] constexpr bool operator==(cref<this_type> other_) const noexcept {
                return _inlineBase == other_._inlineBase &&
                    _externalBase == other_._externalBase &&
                    _idx == other_._idx;
            }

            [[nodiscard]] constexpr bool operator!=(cref<this_type> other_) const noexcept {
                return _inlineBase != other_._inlineBase ||
                    _externalBase != other_._externalBase ||
                    _idx != other_._idx;
            }

            [[nodiscard]] constexpr _STD strong_ordering operator<=>(cref<this_type> other_) const noexcept {

                if (_inlineBase < other_._inlineBase || _externalBase < other_._externalBase) {
                    return _STD strong_ordering::less;
                }

                return _idx <=> other_._idx;
            }
        };

        using Iterator = IteratorImpl<false>;
        using ConstIterator = IteratorImpl<true>;

    protected:
        constexpr void tidy() {

            /* Destroy inline allocated objects */

            _STD _Destroy_range(inline_begin(), _inlineEnd, _alloc);
            _inlineEnd = inline_begin();

            /* Destroy external allocated objects */

            _STD _Destroy_range(external_begin(), _externalEnd, _alloc);
            _traits.deallocate(_alloc, _externalStorage, _STD distance(external_begin(), _externalEnd));

            _externalStorage = _externalEnd = nullptr;
        }

        [[nodiscard]] constexpr const ptr<const Type_> inline_begin() const noexcept {
            return reinterpret_cast<ptr<const Type_>>(&_inlineStorage[0]);
        }

        [[nodiscard]] constexpr const ptr<Type_> inline_begin() noexcept {
            return reinterpret_cast<ptr<Type_>>(&_inlineStorage[0]);
        }

        [[nodiscard]] constexpr ptr<Type_> inline_last() const noexcept {
            return _inlineEnd != inline_begin() ? _inlineEnd - 1 : _inlineEnd;
        }

        [[nodiscard]] constexpr size_type inline_size() const noexcept {
            return _inlineEnd - inline_begin();
        }

        [[nodiscard]] constexpr ptr<Type_> external_begin() const noexcept {
            return _externalStorage;
        }

        [[nodiscard]] constexpr size_t external_size() const noexcept {
            return _externalEnd - external_begin();
        }

    protected:
        constexpr bool inline_emplace_back(auto&&... args_) {

            const auto diff = inline_size();
            if (diff >= Capacity_) {
                return false;
            }

            _traits.template construct<Type_, decltype(args_)...>(
                _alloc,
                _inlineEnd,
                _STD forward<decltype(args_)>(args_)...
            );
            return ++_inlineEnd;
        }

        constexpr void inline_pop_back() {
            assert(_inlineEnd != inline_begin());
            _traits.destroy(_alloc, --_inlineEnd);
        }

        constexpr void inline_erase_at(size_type where_) {

            auto it = inline_begin() + where_ + 1;
            const auto end = _inlineEnd;

            if constexpr (_STD is_nothrow_move_assignable_v<Type_> || not _STD is_copy_assignable_v<Type_>) {
                for (; it != end; ++it) {
                    *(it - 1) = _STD move(*it);
                }
            } else {
                for (; it != end; ++it) {
                    *(it - 1) = *it;
                }
            }

            inline_pop_back();
        }

        constexpr void _Xchange_external(const ptr<Type_> newPt_, const size_type newSize_) {

            if (external_begin()) {
                _STD _Destroy_range(external_begin(), _externalEnd, _alloc);
                _alloc.deallocate(_externalStorage, external_size());
            }

            _externalStorage = newPt_;
            _externalEnd = newPt_ + newSize_;
        }

        constexpr void external_realloc_emplace(auto&&... args_) {

            const auto oldSize = external_size();
            const auto newSize = oldSize + 1;

            auto newPt = _traits.allocate(_alloc, newSize);

            if (external_begin()) {
                if constexpr (_STD is_nothrow_move_constructible_v<Type_> || not _STD is_copy_constructible_v<Type_>) {
                    _STD _Uninitialized_move(external_begin(), _externalEnd, newPt, _alloc);
                } else {
                    _STD _Uninitialized_copy(external_begin(), _externalEnd, newPt, _alloc);
                }
            }

            _traits.construct(_alloc, newPt + oldSize, _STD forward<decltype(args_)>(args_)...);
            _Xchange_external(newPt, newSize);
        }

        constexpr void external_realloc_pop() {

            const auto oldSize = external_size();
            const auto newSize = oldSize - 1;

            if (newSize == 0) {
                _Xchange_external(nullptr, 0);
                return;
            }

            /**/

            const auto oldLast = _externalEnd - 1;
            auto newPt = _traits.allocate(_alloc, newSize);

            if constexpr (_STD is_nothrow_move_constructible_v<Type_> || not _STD is_copy_constructible_v<Type_>) {
                _STD _Uninitialized_move(external_begin(), oldLast, newPt, _alloc);
            } else {
                _STD _Uninitialized_copy(external_begin(), oldLast, newPt, _alloc);
            }

            //_traits.destroy(_alloc, oldLast);
            _Xchange_external(newPt, newSize);
        }

        [[nodiscard]] constexpr bool external_pop_back() {
            if (_externalStorage) {
                external_realloc_pop();
                return true;
            }

            return false;
        }

        constexpr void external_erase_at(size_type where_) {

            const auto oldSize = external_size();
            const auto newSize = oldSize - 1;

            if (newSize == 0) {
                _Xchange_external(nullptr, 0);
                return;
            }

            /**/

            const auto prevBegin = external_begin();
            auto newPt = _traits.allocate(_alloc, newSize);

            if constexpr (_STD is_nothrow_move_constructible_v<Type_> || not _STD is_copy_constructible_v<Type_>) {
                _STD _Uninitialized_move(prevBegin, prevBegin + where_, newPt, _alloc);
                _STD _Uninitialized_move(prevBegin + where_ + 1, _externalEnd, newPt, _alloc);
            } else {
                _STD _Uninitialized_copy(prevBegin, prevBegin + where_, newPt, _alloc);
                _STD _Uninitialized_copy(prevBegin + where_ + 1, _externalEnd, newPt, _alloc);
            }

            //_traits.destroy(_alloc, oldLast);
            _Xchange_external(newPt, newSize);
        }

    public:
        [[nodiscard]] constexpr bool empty() const noexcept {
            return size() == 0;
        }

        [[nodiscard]] constexpr size_type size() const noexcept {
            return inline_size() + external_size();
        }

        [[nodiscard]] constexpr auto begin() const noexcept {
            return ConstIterator { inline_begin(), _externalStorage };
        }

        [[nodiscard]] constexpr auto begin() noexcept {
            return Iterator { inline_begin(), _externalStorage };
        }

        [[nodiscard]] constexpr auto cbegin() noexcept {
            return ConstIterator { inline_begin(), _externalStorage };
        }

        [[nodiscard]] constexpr auto end() const noexcept {
            return ConstIterator { inline_begin(), _externalStorage, static_cast<difference_type>(size()) };
        }

        [[nodiscard]] constexpr auto end() noexcept {
            return Iterator { inline_begin(), _externalStorage, static_cast<difference_type>(size()) };
        }

        [[nodiscard]] constexpr auto cend() noexcept {
            return ConstIterator { inline_begin(), _externalStorage, static_cast<difference_type>(size()) };
        }

    public:
        [[nodiscard]] cref<Type_> front() const noexcept {
            return *inline_begin();
        }

        [[nodiscard]] ref<Type_> front() noexcept {
            return *inline_begin();
        }

        [[nodiscard]] cref<Type_> back() const noexcept {
            if (_externalEnd != nullptr) {
                return *(_externalEnd - 1uLL);
            }
            return *inline_last();
        }

        [[nodiscard]] ref<Type_> back() noexcept {
            if (_externalEnd != nullptr) {
                return *(_externalEnd - 1uLL);
            }
            return *inline_last();
        }

    public:
        constexpr void emplace_back(auto&&... val_) {

            if (inline_emplace_back(_STD forward<decltype(val_)>(val_)...)) {
                return;
            }

            external_realloc_emplace(_STD forward<decltype(val_)>(val_)...);
        }

        constexpr void pop_back() {

            if (external_pop_back()) {
                return;
            }

            inline_pop_back();
        }

        constexpr Iterator erase(cref<ConstIterator> where_) noexcept {
            assert(where_ >= cbegin());

            if (where_._idx < element_capacity) {
                inline_erase_at(where_._idx);
                return;
            }

            external_erase_at(where_.idx - element_capacity);
        }

        constexpr void clear() {
            tidy();
        }

        constexpr void shrink(size_type size_);

        constexpr void resize(size_t newSize_, auto&&... args_);
    };
}
