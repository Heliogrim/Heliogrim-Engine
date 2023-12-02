#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include "../Symbol/Symbol.hpp"

namespace hg::engine::gfx::render::graph {
    struct Requirement final {
        smr<const Symbol> symbol;
    };

    [[nodiscard]] constexpr bool operator==(cref<Requirement> left_, cref<Requirement> right_) noexcept {
        return left_.symbol == right_.symbol;
    }
}

namespace std {
    template <>
    struct hash<::hg::engine::gfx::render::graph::Requirement> :
        public hash<decltype(::hg::engine::gfx::render::graph::Requirement::symbol)> {
        [[nodiscard]] size_t operator()(
            const ::hg::engine::gfx::render::graph::Requirement& val_
        ) const noexcept {
            return static_cast<::hg::cref<hash<decltype(::hg::engine::gfx::render::graph::Requirement::symbol)>>>(*
                this)(
                val_.symbol
            );
        }
    };
}
