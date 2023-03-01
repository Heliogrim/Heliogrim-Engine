#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Hash/Murmur3.hpp>

namespace hg::engine::scene {
    struct SceneNodePath {
    public:
        using this_type = SceneNodePath;

        using decision_type = _STD uint_fast8_t;
        using path_type = u64;

        constexpr static path_type decision_mask = 0b0000'0111;
        constexpr static path_type decision_occupied_mask = 0b0000'1000;

        constexpr static size_t decision_padding = 0b0;
        constexpr static size_t decision_occupied = 0b1;
        constexpr static size_t decision_size = 4u;

    public:
        path_type path;

        /**
         * Constructor
         *
         * @author Julius
         * @date 10.07.2022
         */
        constexpr SceneNodePath() noexcept :
            path() {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 10.07.2022
         *
         * @param   path_ The scene node paths value.
         */
        constexpr SceneNodePath(cref<decltype(path)> path_) noexcept :
            path(path_) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 10.07.2022
         *
         * @param   other_ The scene node path to copy from.
         */
        constexpr SceneNodePath(cref<SceneNodePath> other_) noexcept = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 10.07.2022
         *
         * @param   other_ The scene node path to move from.
         */
        constexpr SceneNodePath(mref<SceneNodePath> other_) noexcept = default;

    public:
        ref<this_type> operator=(cref<this_type> other_) noexcept = default;

        ref<this_type> operator=(mref<this_type> other_) noexcept = default;

    public:
        /**
         * Will return the current path decision
         *
         * @author Julius
         * @date 10.07.2022
         *
         * @returns Bitwise the current path decision encoded with suitable type.
         */
        [[nodiscard]] decision_type cur() const noexcept;

        /**
         * Will move to the next path segment and return the current path decision
         *
         * @author Julius
         * @date 10.07.2022
         *
         * @returns Bitwise the current path decision encoded with suitable type.
         */
        [[nodiscard]] decision_type pop();

        /**
         * Will prepend the provided decision to the current path
         *
         * @author Julius
         * @date 10.07.2022
         *
         * @param   decision_ The decision to prepend to the current path
         *
         * @returns A reference to this object.
         */
        ref<SceneNodePath> push(const decision_type decision_);

        /**
         * Will return the next path decision
         *
         * @author Julius
         * @date 10.07.2022
         *
         * @returns Bitwise the next path decision encoded with suitable type.
         */
        [[nodiscard]] decision_type peek() const noexcept;

    public:
        [[nodiscard]] SceneNodePath inversed() const noexcept;
    };

    FORCE_INLINE bool operator==(const SceneNodePath& lhs_, const SceneNodePath& rhs_) {
        return lhs_.path == rhs_.path;
    }

    FORCE_INLINE bool operator!=(const SceneNodePath& lhs_, const SceneNodePath& rhs_) {
        return lhs_.path != rhs_.path;
    }
}

namespace std {
    template <>
    struct equal_to<hg::engine::scene::SceneNodePath> {
        using value_type = hg::engine::scene::SceneNodePath;

        [[nodiscard]] bool operator()(
            ::hg::cref<value_type> left_,
            ::hg::cref<value_type> right_
        ) const noexcept {
            return left_.path == right_.path;
        }
    };

    template <>
    struct less<hg::engine::scene::SceneNodePath> {
        using value_type = hg::engine::scene::SceneNodePath;

        [[nodiscard]] bool operator()(
            ::hg::cref<value_type> left_,
            ::hg::cref<value_type> right_
        ) const noexcept {
            return left_.path < right_.path;
        }
    };

    template <>
    struct hash<hg::engine::scene::SceneNodePath> {
        using value_type = hg::engine::scene::SceneNodePath;

        [[nodiscard]] _STD size_t operator()(::hg::cref<value_type> value_) const noexcept {
            /**
             * !important! don's use identity
             */
            hg::u64 dst[2];
            hg::hash::murmur3_x64_128(
                &value_,
                sizeof(hg::engine::scene::SceneNodePath),
                0x9FB21C651E98DF25ui32,
                &dst
            );

            return *dst;
        }
    };
}
