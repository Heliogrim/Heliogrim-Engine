#pragma once

#include <concepts>
#include <xutility>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "RenderCommandBufferIterator.hpp"

namespace hg::engine::gfx::render::cmd {
    class RenderCommand;
    class RenderCommandBuffer;
    class PooledRenderCommandBuffer;
}

namespace hg::engine::gfx::render::cmd {
    class RenderCommandBufferIterator;

    template <typename Type_>
    concept IsRCBIterator = _STD derived_from<Type_, RenderCommandBufferIterator> && _STD random_access_iterator<Type_>;

    class RenderCommandBufferIterator {
    public:
        using this_type = RenderCommandBufferIterator;

        using range_type = const Vector<ptr<const RenderCommand>>;
        using range_iterator = range_type::iterator;

        using iterator_category = range_iterator::iterator_category;
        using iterator_concept = range_iterator::iterator_concept;

        using size_type = range_type::size_type;
        using difference_type = range_iterator::difference_type;

        using pointer = range_iterator::pointer;
        using reference = range_iterator::reference;
        using value_type = range_iterator::value_type;

    public:
        constexpr RenderCommandBufferIterator() noexcept :
            _rangeIt() {}

        constexpr RenderCommandBufferIterator(mref<range_iterator> rangeIt_) noexcept :
            _rangeIt(_STD move(rangeIt_)) {}

        constexpr RenderCommandBufferIterator(mref<this_type> other_) noexcept :
            _rangeIt(_STD move(other_._rangeIt)) {}

        constexpr RenderCommandBufferIterator(cref<this_type> other_) noexcept = default;

        constexpr ~RenderCommandBufferIterator() noexcept = default;

    public:
        constexpr ref<this_type> operator=(mref<this_type> other_) noexcept = default;

        constexpr ref<this_type> operator=(cref<this_type> other_) noexcept = default;

    private:
        range_iterator _rangeIt;

    public:
        [[nodiscard]] constexpr reference operator*() const noexcept {
            return *_rangeIt;
        }

        [[nodiscard]] constexpr pointer operator->() const noexcept {
            return _rangeIt.operator->();
        }

    public:
        constexpr ref<this_type> operator+=(difference_type offset_) noexcept {
            _rangeIt += offset_;
            return *this;
        }

        constexpr ref<this_type> operator-=(difference_type offset_) noexcept {
            _rangeIt -= offset_;
            return *this;
        }

        [[nodiscard]] constexpr this_type operator+(difference_type offset_) const noexcept {
            auto tmp { *this };
            tmp += offset_;
            return tmp;
        }

        [[nodiscard]] constexpr this_type operator-(difference_type offset_) const noexcept {
            auto tmp { *this };
            tmp -= offset_;
            return tmp;
        }

        constexpr ref<this_type> operator++() noexcept {
            ++_rangeIt;
            return *this;
        }

        [[nodiscard]] constexpr this_type operator++() const noexcept {
            auto tmp { *this };
            ++tmp;
            return tmp;
        }

        constexpr ref<this_type> operator--() noexcept {
            --_rangeIt;
            return *this;
        }

        [[nodiscard]] constexpr this_type operator--() const noexcept {
            auto tmp { *this };
            --tmp;
            return tmp;
        }

        [[nodiscard]] constexpr this_type operator++(int) noexcept {
            auto tmp { *this };
            ++*this;
            return tmp;
        }

        [[nodiscard]] constexpr this_type operator--(int) noexcept {
            auto tmp { *this };
            --*this;
            return tmp;
        }

        [[nodiscard]] constexpr reference operator[](difference_type offset_) const noexcept {
            auto tmp { *this };
            tmp += offset_;
            return *tmp;
        }

    public:
        [[nodiscard]] constexpr difference_type operator-(cref<this_type> other_) const noexcept {
            return _rangeIt - other_._rangeIt;
        }

    public:
        [[nodiscard]] constexpr bool operator==(cref<this_type> other_) const noexcept {
            return _rangeIt == other_._rangeIt;
        }

        [[nodiscard]] constexpr _STD strong_ordering operator<=>(cref<this_type> other_) const {
            return _rangeIt <=> other_._rangeIt;
        }
    };

    [[nodiscard]] constexpr RenderCommandBufferIterator operator+(
        const _STD iter_difference_t<RenderCommandBufferIterator> diff_,
        RenderCommandBufferIterator next_
    ) noexcept {
        next_ += diff_;
        return next_;
    }

    static_assert(IsRCBIterator<RenderCommandBufferIterator>);
}
