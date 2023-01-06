#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::scheduler {
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
    struct hash<::ember::engine::scheduler::CompositeDependency> :
        _STD hash<::ember::ptr<void>> {
        [[nodiscard]] size_t operator()(
            ::ember::cref<::ember::engine::scheduler::CompositeDependency> value_
        ) const noexcept {
            return static_cast<::ember::cref<_STD hash<::ember::ptr<void>>>>(*this)(value_.stage);
        }
    };
}
