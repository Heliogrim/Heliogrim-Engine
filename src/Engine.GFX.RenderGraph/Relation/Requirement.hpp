#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Description.hpp"

namespace hg::engine::gfx::render::graph {
    struct Requirement final {
        size_t identifier;
        smr<Description> description;
    };

    [[nodiscard]] constexpr bool operator==(cref<Requirement> left_, cref<Requirement> right_) noexcept {
        return left_.identifier == right_.identifier;
    }
}

namespace std {
    template <>
    struct hash<::hg::engine::gfx::render::graph::Requirement> {
        [[nodiscard]] constexpr size_t operator()(
            const ::hg::engine::gfx::render::graph::Requirement& val_
        ) const noexcept {
            return val_.identifier;
        }
    };
}
