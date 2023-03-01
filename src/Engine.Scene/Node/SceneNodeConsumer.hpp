#pragma once

#include <concepts>
#include <functional>

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::scene {
    template <typename Consumable>
    struct SceneNodeConsumer {
        [[nodiscard]] bool operator()(const ptr<Consumable> entry_) noexcept {
            return false;
        }
    };

    template <typename Type, typename Consumable>
    concept IsSceneNodeConsumer = std::convertible_to<Type, _STD function<bool(Consumable)>>;

    template <typename Consumable>
    struct SceneNodeBatchConsumer {
        [[nodiscard]] bool operator()(u32 batchIdx_, const ptr<Consumable> entry_) noexcept {
            return false;
        }
    };

    template <typename Type, typename Consumable>
    concept IsSceneNodeBatchConsumer = std::convertible_to<Type, _STD function<bool(u32, Consumable)>>;
}
