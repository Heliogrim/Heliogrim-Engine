#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include "../Symbol/Symbol.hpp"

namespace hg::engine::gfx::render::graph {
    struct Provision final {
        smr<const Symbol> symbol;
    };

    [[nodiscard]] constexpr bool operator==(cref<Provision> left_, cref<Provision> right_) noexcept {
        return left_.symbol == right_.symbol;
    }
}

namespace std {
    template <>
    struct hash<::hg::engine::gfx::render::graph::Provision> :
        public hash<decltype(::hg::engine::gfx::render::graph::Provision::symbol)> {
        [[nodiscard]] size_t operator()(
            const ::hg::engine::gfx::render::graph::Provision& val_
        ) const noexcept {
            return static_cast<::hg::cref<hash<decltype(::hg::engine::gfx::render::graph::Provision::symbol)>>>(*this)(
                val_.symbol
            );
        }
    };
}
