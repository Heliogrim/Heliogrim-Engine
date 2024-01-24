#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::scheduler {
    struct CompositeDependency {
        non_owning_rptr<struct CompositeStage> stage;
        u64 requiredGeneration;

    public:
        [[nodiscard]] bool operator==(cref<CompositeDependency> other_) const noexcept {
            return stage == other_.stage;
        }

        [[nodiscard]] bool operator!=(cref<CompositeDependency> other_) const noexcept {
            return stage != other_.stage;
        }
    };
}

namespace std {
    template <>
    struct hash<::hg::engine::scheduler::CompositeDependency> :
        std::hash<::hg::ptr<void>> {
        [[nodiscard]] size_t operator()(
            ::hg::cref<::hg::engine::scheduler::CompositeDependency> value_
        ) const noexcept {
            return static_cast<::hg::cref<std::hash<::hg::ptr<void>>>>(*this)(value_.stage);
        }
    };
}
