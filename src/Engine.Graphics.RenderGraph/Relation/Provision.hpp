#pragma once

#include <Engine.Common/__macro.hpp>

#include "../Symbol/Symbol.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::graph {
    struct Provision final {
        smr<const Symbol> symbol;
    };

    [[nodiscard]] constexpr bool operator==(cref<Provision> left_, cref<Provision> right_) noexcept {
        return left_.symbol == right_.symbol;
    }
}

namespace std {
    template <>
    struct hash<::hg::engine::render::graph::Provision> :
        public hash<decltype(::hg::engine::render::graph::Provision::symbol)> {
        [[nodiscard]] size_t operator()(
            const ::hg::engine::render::graph::Provision& val_
        ) const noexcept {
            return static_cast<::hg::cref<hash<decltype(::hg::engine::render::graph::Provision::symbol)>>>(*this)(
                val_.symbol
            );
        }
    };
}
