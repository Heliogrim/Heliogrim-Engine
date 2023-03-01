#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::scene {
    /**
     * Forward Declaration
     */
    struct invalid_node_version;

    struct SceneNodeVersion {
        uint_fast64_t data;

        SceneNodeVersion() noexcept;

        SceneNodeVersion(invalid_node_version) noexcept;

        SceneNodeVersion(cref<decltype(data)> data_) noexcept;

        SceneNodeVersion(cref<SceneNodeVersion> other_) noexcept;

        SceneNodeVersion(mref<SceneNodeVersion> other_) noexcept;

        ref<SceneNodeVersion> operator=(cref<SceneNodeVersion> other_) noexcept;

        ref<SceneNodeVersion> operator=(mref<SceneNodeVersion> other_) noexcept;
    };

    struct invalid_node_version {
        static constexpr decltype(SceneNodeVersion::data) bit_sequence = 0;

        constexpr invalid_node_version() = default;

        explicit operator SceneNodeVersion() const noexcept {
            return SceneNodeVersion { bit_sequence };
        }

        constexpr operator uint_fast64_t() const noexcept {
            return bit_sequence;
        }
    };

    [[nodiscard]] constexpr bool operator==(cref<invalid_node_version>, cref<invalid_node_version>) noexcept {
        return true;
    }

    [[nodiscard]] constexpr bool operator!=(cref<invalid_node_version>, cref<invalid_node_version>) noexcept {
        return false;
    }

    [[nodiscard]] inline bool operator==(cref<SceneNodeVersion> left_, cref<SceneNodeVersion> right_) noexcept {
        return left_.data == right_.data;
    }

    [[nodiscard]] inline bool operator!=(cref<SceneNodeVersion> left_, cref<SceneNodeVersion> right_) noexcept {
        return left_.data != right_.data;
    }

    [[nodiscard]] inline bool operator!=(invalid_node_version, cref<SceneNodeVersion> right_) noexcept {
        return invalid_node_version::bit_sequence != right_.data;
    }

    [[nodiscard]] inline bool operator==(invalid_node_version, cref<SceneNodeVersion> right_) noexcept {
        return invalid_node_version::bit_sequence == right_.data;
    }
}
