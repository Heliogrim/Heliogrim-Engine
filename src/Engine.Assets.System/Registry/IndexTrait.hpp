#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <concepts>

#include "../__fwd.hpp"

namespace hg::engine::assets::system {
    template <typename Index_>
    struct IndexTrait {
        using this_type = IndexTrait<Index_>;
        using trait_type = this_type;
        using index_type = Index_;

        /**/

        static constexpr bool dynamic = index_type::dynamic::value;

        /**/

        static constexpr bool unique = index_type::unique::value;
        static constexpr bool multiple = index_type::multiple::value;

        using multiple_options_type = _STD conditional_t<
            _STD negation_v<_STD is_void<typename index_type::multiple_options_type>>,
            typename index_type::multiple_options_type,
            void>;

        /**/

        using data_type = typename index_type::data_type;
        using hash_type = _STD hash<data_type>;
        using projection_type = decltype(index_type::proj);
        using comparator_type = decltype(index_type::comp);

        /**/

        [[nodiscard]] static data_type project(const non_owning_rptr<const Asset> asset_) noexcept {
            return (index_type::proj)(asset_);
        }

        template <typename Type_ = data_type> requires _STD is_trivially_copy_constructible_v<Type_>
        [[nodiscard]] static _STD size_t hash(const Type_ value_) noexcept {
            return hash_type {}(value_);
        }

        template <typename Type_ = data_type> requires _STD is_move_constructible_v<Type_>
        [[nodiscard]] static _STD size_t hash(mref<Type_> value_) noexcept {
            return hash_type {}(_STD move(value_));
        }

        [[nodiscard]] static _STD size_t hash(const non_owning_rptr<const Asset> asset_) noexcept {
            return this_type::template hash<data_type>(this_type::project(asset_));
        }

        /**/

        template <typename Type_ = data_type, typename Options_ = multiple_options_type> requires
            _STD is_trivially_copy_constructible_v<Type_> &&
            _STD is_void_v<Options_>
        [[nodiscard]] static _STD strong_ordering compare(
            const Type_ left_,
            const Type_ right_
        ) noexcept {
            return (index_type::comp)(left_, right_);
        }

        template <typename Type_ = data_type, typename Options_ = multiple_options_type> requires
            (!_STD is_trivially_copy_constructible_v<Type_>) &&
            _STD is_void_v<Options_>
        [[nodiscard]] static _STD strong_ordering compare(
            cref<Type_> left_,
            cref<Type_> right_
        ) noexcept {
            return (index_type::comp)(left_, right_);
        }

        template <typename Type_ = data_type, typename Options_ = multiple_options_type> requires
            _STD is_trivially_copy_constructible_v<Type_> &&
            (!_STD is_void_v<Options_>)
        [[nodiscard]] static _STD strong_ordering compare(
            const Type_ left_,
            const Type_ right_,
            cref<Options_> options_
        ) noexcept {
            return (index_type::comp)(left_, right_, options_);
        }

        template <typename Type_ = data_type, typename Options_ = multiple_options_type> requires
            (!_STD is_trivially_copy_constructible_v<Type_>) &&
            (!_STD is_void_v<Options_>)
        [[nodiscard]] static _STD strong_ordering compare(
            cref<Type_> left_,
            cref<Type_> right_,
            cref<Options_> options_
        ) noexcept {
            return (index_type::comp)(left_, right_, options_);
        }
    };
}
