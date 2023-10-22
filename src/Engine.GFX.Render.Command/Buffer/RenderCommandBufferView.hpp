#pragma once

#include <iterator>

#include "RenderCommandBufferIterator.hpp"

namespace hg::engine::gfx::render::cmd {
    struct RenderCommandBufferView {
        RenderCommandBufferIterator beginIt;
        RenderCommandBufferIterator endIt;

        [[nodiscard]] constexpr bool empty() const noexcept {
            return beginIt == endIt;
        }

        [[nodiscard]] constexpr auto size() const noexcept {
            return endIt - beginIt;
        }

        [[nodiscard]] constexpr auto begin() const noexcept {
            return beginIt;
        }

        [[nodiscard]] constexpr auto end() const noexcept {
            return endIt;
        }

        [[nodiscard]] constexpr auto cbegin() const noexcept {
            return beginIt;
        }

        [[nodiscard]] constexpr auto cend() const noexcept {
            return endIt;
        }

        [[nodiscard]] constexpr auto rbegin() const noexcept {
            return _STD reverse_iterator<RenderCommandBufferIterator> { --endIt };
        }

        [[nodiscard]] constexpr auto rend() const noexcept {
            return _STD reverse_iterator<RenderCommandBufferIterator> { --beginIt };
        }
    };
}
