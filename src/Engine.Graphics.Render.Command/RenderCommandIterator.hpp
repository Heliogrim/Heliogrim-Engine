#pragma once

#include <iterator>
#include <Engine.Common/Wrapper.hpp>

#include "RenderCommand.hpp"

namespace hg::engine::render::cmd {
    class RenderCommandIterator {
    public:
        using this_type = RenderCommandIterator;

        using iterator_concept = std::forward_iterator_tag;
        using iterator_tag = std::forward_iterator_tag;

        using size_type = size_t;
        using difference_type = ptrdiff_t;

        using pointer = ptr<RenderCommand>;
        using reference = ref<RenderCommand>;
        using const_reference = cref<RenderCommand>;

    public:
        constexpr RenderCommandIterator() noexcept :
            _iter(nullptr) {}

        constexpr RenderCommandIterator(const ptr<RenderCommand> iter_) :
            _iter(iter_) {}

        constexpr RenderCommandIterator(mref<this_type> other_) noexcept = default;

        constexpr RenderCommandIterator(cref<this_type> other_) noexcept = default;

        constexpr ~RenderCommandIterator() noexcept = default;

    public:
        ref<this_type> operator=(mref<this_type> other_) noexcept = default;

        ref<this_type> operator=(cref<this_type> other_) noexcept = default;

    private:
        nmpt<RenderCommand> _iter;

    public:
        [[nodiscard]] constexpr bool hasNext() const noexcept {
            return _iter->next != nullptr;
        }

        [[nodiscard]] constexpr bool valid() const noexcept {
            return _iter != nullptr;
        }

    public:
        [[nodiscard]] reference operator*() noexcept {
            return *_iter;
        }

        [[nodiscard]] pointer operator->() noexcept {
            return _iter.get();
        }

    public:
        ref<this_type> operator++() noexcept {
            _iter = _iter->next;
            return *this;
        }

        this_type operator++(int) noexcept {
            const auto tmp { *this };
            ++*this;
            return tmp;
        }

        ref<this_type> operator+=(size_type distance_) noexcept {
            while (distance_--) {
                _iter = _iter->next;
            }
            return *this;
        }

        this_type operator+(const size_type distance_) const noexcept {
            auto tmp { *this };
            tmp += distance_;
            return tmp;
        }

        this_type operator[](const size_type distance_) const noexcept {
            return (*this) + distance_;
        }

    public:
        [[nodiscard]] bool operator==(cref<this_type> other_) const noexcept {
            return _iter == other_._iter;
        }

        [[nodiscard]] bool operator!=(cref<this_type> other_) const noexcept {
            return _iter != other_._iter;
        }
    };
}
