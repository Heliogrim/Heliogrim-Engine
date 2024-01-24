#pragma once

#include <cassert>

#include "../Types.hpp"
#include "../Wrapper.hpp"

namespace hg {
    template <class Type_, class Allocator_ = std::allocator<Type_>>
    class UnsafeVector {
    public:
        using this_type = UnsafeVector<Type_, Allocator_>;
        using allocator_type = Allocator_;
        using allocator_traits = std::allocator_traits<allocator_type>;

    public:
        UnsafeVector() :
            _first(nullptr),
            _last(nullptr),
            _end(nullptr) {}

        UnsafeVector(cref<this_type> other_) = delete;

        UnsafeVector(mref<this_type> other_) noexcept :
            _first(std::exchange(other_._first, nullptr)),
            _last(std::exchange(other_._last, nullptr)),
            _end(std::exchange(other_._end, nullptr)),
            _alloc(other_._alloc) {}

        ~UnsafeVector() {
            tidy();
        }

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type> other_) noexcept {

            if (std::addressof(other_) != this) {
                std::swap(_first, other_._first);
                std::swap(_last, other_._last);
                std::swap(_end, other_._end);
                std::swap(_alloc, other_._alloc);
            }

            return *this;
        }

    private:
        ptr<Type_> _first;
        ptr<Type_> _last;
        ptr<Type_> _end;

        [[no_unique_address]] allocator_type _alloc;

    public:
        template <typename Fnc_>
        void each(mref<Fnc_> callback_) {
            for (auto* iter = _last; iter >= _first; --iter) {
                callback_(iter);
            }
        }

        void tidy() {
            clear();
            _alloc.deallocate(_first, capacity());

            _first = nullptr;
            _last = nullptr;
            _end = nullptr;
        }

        template <typename Fnc_>
        void tidy(mref<Fnc_> destructor_) {
            clear<Fnc_>(std::move(destructor_));
            _alloc.deallocate(_first);

            _first = nullptr;
            _last = nullptr;
            _end = nullptr;
        }

        void grow(const u64 newCapacity_) {
            assert(newCapacity_ > capacity());

            auto* newFirst { _alloc.allocate(newCapacity_) };

            if constexpr (std::is_nothrow_move_constructible_v<Type_> || !std::is_copy_constructible_v<Type_>) {
                std::_Uninitialized_move(_first, _last, newFirst, _alloc);
            } else {
                std::_Uninitialized_copy(_first, _last, newFirst, _alloc);
            }

            _alloc.deallocate(_first, capacity());

            _last = newFirst + size();
            _first = newFirst;
            _end = newFirst + newCapacity_;
        }

        template <typename Fnc_>
        void grow(const u64 newCapacity_, mref<Fnc_> destructor_) {
            assert(newCapacity_ > capacity());

            const auto* newFirst { _alloc.allocate(newCapacity_) };

            if constexpr (std::is_nothrow_move_constructible_v<Type_> || !std::is_copy_constructible_v<Type_>) {
                std::_Uninitialized_move(_first, _last, newFirst, _alloc);
            } else {
                std::_Uninitialized_copy(_first, _last, newFirst, _alloc);
            }

            each<Fnc_>(std::move(destructor_));
            _alloc.deallocate(_first, capacity());

            _last = newFirst + size();
            _first = newFirst;
            _end = newFirst + newCapacity_;
        }

    public:
        [[nodiscard]] u64 size() const noexcept {
            return _last - _first;
        }

        [[nodiscard]] bool empty() const noexcept {
            return _last == _first;
        }

        [[nodiscard]] u64 capacity() const noexcept {
            return _end - _first;
        }

    public:
        void push_back(mref<Type_> value_) {

            if (const auto flr { size() }; flr >= capacity()) {
                grow((flr << 1) | 1ui64);
            }

            if constexpr (std::is_nothrow_move_constructible_v<Type_> || !std::is_copy_constructible_v<Type_>) {
                allocator_traits::construct(_alloc, _last++, std::move(value_));
            } else {
                allocator_traits::construct(_alloc, _last++, value_);
            }
        }

        Type_ pop_back() {
            auto tmp { *_last };
            --_last;
            return tmp;
        }

        void clear() {
            _last = _first;
        }

        template <typename Fnc_>
        void clear(mref<Fnc_> destructor_) {
            each<Fnc_>(std::move(destructor_));
            clear();
        }

    public:
        [[nodiscard]] ref<Type_> front() {
            return *_first;
        }

        [[nodiscard]] ref<Type_> back() {
            return *_last;
        }
    };
}
