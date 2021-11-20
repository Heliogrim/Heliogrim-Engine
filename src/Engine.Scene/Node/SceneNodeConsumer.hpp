#pragma once
#include <concepts>
#include <functional>
#include "SceneNode.hpp"

namespace ember::engine::scene {

    template <typename Consumable = SceneNode>
    struct SceneNodeConsumer {
        [[nodiscard]] bool operator()(cref<Consumable> entry_) noexcept {
            return false;
        }
    };

    template <typename Type, typename Consumable = SceneNode>
    concept IsSceneNodeConsumer = std::convertible_to<Type, _STD function<bool(Consumable)>>;

    template <typename Consumable = SceneNode>
    struct SceneNodeBatchConsumer {
        [[nodiscard]] bool operator()(u32 batchIdx_, cref<Consumable> entry_) noexcept {
            return false;
        }
    };

    template <typename Type, typename Consumable = SceneNode>
    concept IsSceneNodeBatchConsumer = std::convertible_to<Type, _STD function<bool(u32, Consumable)>>;
}
