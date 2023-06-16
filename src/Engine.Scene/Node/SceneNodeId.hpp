#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Hash/Murmur3.hpp>

namespace hg::engine::scene {
    /**
     * Forward Declaration
     */
    struct invalid_node_id;

    struct SceneNodeId {
        u64 data;

        /**
         * Constructor
         *
         * @author Julius
         * @date 15.08.2021
         */
        constexpr SceneNodeId(cref<invalid_node_id>);

        /**
         * Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  data_ The data.
         */
        constexpr SceneNodeId(cref<decltype(data)> data_) :
            data(data_) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         */
        constexpr SceneNodeId(cref<SceneNodeId> other_) noexcept = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         */
        constexpr SceneNodeId(mref<SceneNodeId> other_) noexcept;

        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        constexpr ref<SceneNodeId> operator=(cref<SceneNodeId> other_) noexcept = default;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        constexpr ref<SceneNodeId> operator=(mref<SceneNodeId> other_) noexcept;

        /**
         * Cast that converts the given to a bool
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns The result of the operation.
         */
        FORCE_INLINE operator bool() const noexcept;

        /**
         * Logical negation of boolish representation
         *
         * @author Julius
         * @date 16.08.2021
         */
        FORCE_INLINE bool operator!() const noexcept;
    };

    /**
     * Equality operator
     *
     * @author Julius
     * @date 15.08.2021
     *
     * @param  left_ The first instance to compare.
     * @param  right_ The second instance to compare.
     *
     * @returns True if the parameters are considered equivalent.
     */
    FORCE_INLINE bool operator==(cref<SceneNodeId> left_, cref<SceneNodeId> right_) noexcept {
        return left_.data == right_.data;
    }

    /**
     * Inequality operator
     *
     * @author Julius
     * @date 15.08.2021
     *
     * @param  left_ The first instance to compare.
     * @param  right_ The second instance to compare.
     *
     * @returns True if the parameters are not considered equivalent.
     */
    FORCE_INLINE bool operator!=(cref<SceneNodeId> left_, cref<SceneNodeId> right_) noexcept {
        return left_.data != right_.data;
    }

    /**
     * An invalid node identifier.
     *
     * @author Julius
     * @date 15.08.2021
     */
    struct invalid_node_id {
        static constexpr decltype(SceneNodeId::data) bit_sequence = 0x0;

        constexpr invalid_node_id() noexcept = default;

        constexpr operator SceneNodeId() const noexcept {
            return SceneNodeId { invalid_node_id::bit_sequence };
        }

        constexpr operator u64() const noexcept {
            return invalid_node_id::bit_sequence;
        }

        constexpr operator bool() const noexcept {
            return false;
        }
    };

    /**
     * Constructor Conversion with invalid node id
     *
     * @author Julius
     * @date 15.08.2021
     */
    constexpr SceneNodeId::SceneNodeId(cref<invalid_node_id>) :
        data(invalid_node_id::bit_sequence) {}

    /**
     * Move Constructor
     *
     * @author Julius
     * @date 15.08.2021
     *
     * @param  other_ The other.
     */
    constexpr SceneNodeId::SceneNodeId(mref<SceneNodeId> other_) noexcept :
        data(_STD exchange(other_.data, invalid_node_id::bit_sequence)) {}

    /**
     * Move Assignment operator
     *
     * @author Julius
     * @date 15.08.2021
     *
     * @param  other_ The other.
     *
     * @returns A shallow copy of this.
     */
    constexpr ref<SceneNodeId> SceneNodeId::operator=(mref<SceneNodeId> other_) noexcept {
        if (this != &other_) {
            data = _STD exchange(other_.data, invalid_node_id::bit_sequence);
        }

        return *this;
    }

    /**
     * Equality operator for invalid node ids
     *
     * @author Julius
     * @date 15.08.2021
     *
     * @returns True if the parameters are considered equivalent.
     */
    [[nodiscard]] constexpr FORCE_INLINE bool operator==(cref<invalid_node_id>, cref<invalid_node_id>) noexcept {
        return true;
    }

    /**
     * Inequality operator for invalid node ids
     *
     * @author Julius
     * @date 15.08.2021
     *
     * @returns True if the parameters are not considered equivalent.
     */
    [[nodiscard]] constexpr FORCE_INLINE bool operator!=(cref<invalid_node_id>, cref<invalid_node_id>) noexcept {
        return false;
    }

    /**
     * Equality operator for invalid node id and valid one
     *
     * @author Julius
     * @date 15.08.2021
     *
     * @param  left_ The first instance to compare.
     * @param  right_ The second instance to compare.
     *
     * @returns True if the parameters are considered equivalent.
     */
    [[nodiscard]] FORCE_INLINE bool operator==(cref<invalid_node_id> left_, cref<SceneNodeId> right_) noexcept {
        return left_.operator SceneNodeId() == right_;
    }

    /**
     * Inequality operator for invalid node id and valid one
     *
     * @author Julius
     * @date 15.08.2021
     *
     * @param  left_ The first instance to compare.
     * @param  right_ The second instance to compare.
     *
     * @returns True if the parameters are not considered equivalent.
     */
    [[nodiscard]] FORCE_INLINE bool operator!=(cref<invalid_node_id> left_, cref<SceneNodeId> right_) noexcept {
        return left_.operator SceneNodeId() != right_;
    }

    [[nodiscard]] FORCE_INLINE SceneNodeId::operator bool() const noexcept {
        return this->data != invalid_node_id::bit_sequence;
    }

    [[nodiscard]] FORCE_INLINE bool SceneNodeId::operator!() const noexcept {
        return this->data == invalid_node_id::bit_sequence;
    }
}

namespace std {
    /**
     * Specific equality comparison for SceneNodeId
     *
     * @author Julius
     * @date 16.08.2021
     */
    template <>
    struct equal_to<hg::engine::scene::SceneNodeId> {
        using const_reference_type = hg::cref<hg::engine::scene::SceneNodeId>;

        [[nodiscard]] bool operator()(const_reference_type left_, const_reference_type right_) const noexcept {
            return left_.data == right_.data;
        }
    };

    /**
     * Specific less comparison for SceneNodeId
     *
     * @author Julius
     * @date 16.08.2021
     */
    template <>
    struct less<hg::engine::scene::SceneNodeId> {
        using const_reference_type = hg::cref<hg::engine::scene::SceneNodeId>;

        [[nodiscard]] bool operator()(const_reference_type left_, const_reference_type right_) const noexcept {
            return left_.data < right_.data;
        }
    };

    /**
     * Specific hash for SceneNodeId
     *
     * @author Julius
     * @date 16.08.2021
     */
    template <>
    struct hash<hg::engine::scene::SceneNodeId> {
        using const_reference_type = hg::cref<hg::engine::scene::SceneNodeId>;

        [[nodiscard]] _STD size_t operator()(const_reference_type value_) const noexcept {
            /**
             * !important! don't use identity
             */
            hg::u64 dst[2];
            hg::hash::murmur3_x64_128(
                &value_,
                sizeof(hg::engine::scene::SceneNodeId),
                0x9FB21C651E98DF25ui32,
                &dst
            );

            return dst[0];
        }
    };
}
