#pragma once

#include <cstdint>
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::scene {

    /**
     * Forward Declaration
     */
    struct invalid_node_version;

    struct SceneNodeVersion {
        uint_fast64_t data;

        constexpr SceneNodeVersion() noexcept;

        constexpr SceneNodeVersion(cref<invalid_node_version>)
        noexcept;

        constexpr SceneNodeVersion(cref<decltype(data)> data_) noexcept :
            data(data_) {}

        constexpr SceneNodeVersion(cref<SceneNodeVersion> other_) noexcept = default;

        constexpr SceneNodeVersion(mref<SceneNodeVersion> other_) noexcept;

        constexpr ref<SceneNodeVersion> operator=(cref<SceneNodeVersion> other_) noexcept = default;

        constexpr ref<SceneNodeVersion> operator=(mref<SceneNodeVersion> other_) noexcept;
    };

    struct invalid_node_version {

        static constexpr decltype(SceneNodeVersion::data) bit_sequence = 0;

        constexpr invalid_node_version() {}

        constexpr operator SceneNodeVersion() const noexcept {
            return SceneNodeVersion { bit_sequence };
        }

        constexpr operator uint_fast64_t() const noexcept {
            return bit_sequence;
        }
    };

    constexpr SceneNodeVersion::SceneNodeVersion() noexcept :
        data(invalid_node_version::bit_sequence) { }

    constexpr SceneNodeVersion::SceneNodeVersion(cref<invalid_node_version>) noexcept :
        data(invalid_node_version::bit_sequence) { }

    constexpr SceneNodeVersion::SceneNodeVersion(mref<SceneNodeVersion> other_) noexcept :
        data(_STD exchange(other_.data, invalid_node_version::bit_sequence)) { }

    constexpr ref<SceneNodeVersion> SceneNodeVersion::operator=(mref<SceneNodeVersion> other_) noexcept {
        if (this != &other_) {
            data = _STD exchange(other_.data, invalid_node_version::bit_sequence);
        }

        return *this;
    }

    [[nodiscard]] constexpr bool operator==(cref<invalid_node_version>, cref<invalid_node_version>) noexcept {
        return true;
    }

    [[nodiscard]] constexpr bool operator!=(cref<invalid_node_version>, cref<invalid_node_version>) noexcept {
        return false;
    }

    [[nodiscard]] constexpr bool operator==(cref<SceneNodeVersion> left_, cref<SceneNodeVersion> right_) noexcept {
        return left_.data == right_.data;
    }

    [[nodiscard]] constexpr bool operator!=(cref<SceneNodeVersion> left_, cref<SceneNodeVersion> right_) noexcept {
        return left_.data != right_.data;
    }

    [[nodiscard]] constexpr bool operator!=(invalid_node_version, cref<SceneNodeVersion> right_) noexcept {
        return invalid_node_version::bit_sequence != right_.data;
    }

    [[nodiscard]] constexpr bool operator==(invalid_node_version, cref<SceneNodeVersion> right_) noexcept {
        return invalid_node_version::bit_sequence == right_.data;
    }

}
