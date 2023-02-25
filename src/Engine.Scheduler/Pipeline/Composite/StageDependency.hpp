#pragma once

#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Math/Hash.hpp>

namespace ember::engine::scheduler {
    struct StageDependency final {
        CompactSet<non_owning_rptr<const Stage>> required;

        non_owning_rptr<class StagePipeline> pipeline;
        non_owning_rptr<const Stage> stage;
    };
}

namespace std {
    template <>
    struct hash<::ember::engine::scheduler::StageDependency> :
        public _STD hash<::ember::ptr<void>>,
        public _STD hash<::ember::ptr<const void>> {
        [[nodiscard]] size_t operator()(
            ::ember::cref<::ember::engine::scheduler::StageDependency> value_
        ) const noexcept {
            size_t seed = 0;
            ::ember::hash::hash_combine(
                seed,
                static_cast<::ember::cref<_STD hash<::ember::ptr<void>>>>(*this)(value_.pipeline)
            );
            ::ember::hash::hash_combine(
                seed,
                static_cast<::ember::cref<_STD hash<::ember::ptr<const void>>>>(*this)(value_.stage)
            );
            return seed;
        }
    };

    template <>
    struct equal_to<::ember::engine::scheduler::StageDependency> {
        [[nodiscard]] bool operator()(
            ::ember::cref<::ember::engine::scheduler::StageDependency> lhs_,
            ::ember::cref<::ember::engine::scheduler::StageDependency> rhs_
        ) const noexcept {
            return lhs_.stage == rhs_.stage && lhs_.pipeline == rhs_.pipeline;
        }
    };
}
