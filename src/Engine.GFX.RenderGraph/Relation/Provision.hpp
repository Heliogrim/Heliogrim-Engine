#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "Description.hpp"

namespace hg::engine::gfx::render::graph {
    struct Provision final {
        size_t identifier;
        smr<Description> description;
    };

    [[nodiscard]] constexpr bool operator==(cref<Provision> left_, cref<Provision> right_) noexcept {
        return left_.identifier == right_.identifier;
    }
}

namespace std {
    template <>
    struct hash<::hg::engine::gfx::render::graph::Provision> {
        [[nodiscard]] constexpr size_t operator()(
            const ::hg::engine::gfx::render::graph::Provision& val_
        ) const noexcept {
            return val_.identifier;
        }
    };
}
