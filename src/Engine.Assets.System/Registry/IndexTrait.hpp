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

        using multiple_options_type = std::conditional_t<
            std::negation_v<std::is_void<typename index_type::multiple_options_type>>,
            typename index_type::multiple_options_type,
            void>;

        /**/

        using data_type = typename index_type::data_type;
        using hash_type = std::hash<data_type>;

        using projection_type = typename index_type::proj_type;
        using look_comp_type = typename index_type::look_type;
        using relation_type = typename index_type::relation_type;

        /**/

        [[nodiscard]] static data_type project(const non_owning_rptr<const Asset> asset_) noexcept {
            return (index_type::proj)(asset_);
        }

        template <typename Type_ = data_type> requires std::is_trivially_copy_constructible_v<Type_>
        [[nodiscard]] static std::size_t hash(const Type_ value_) noexcept {
            return hash_type {}(value_);
        }

        template <typename Type_ = data_type> requires std::is_move_constructible_v<Type_>
        [[nodiscard]] static std::size_t hash(mref<Type_> value_) noexcept {
            return hash_type {}(std::move(value_));
        }

        [[nodiscard]] static std::size_t hash(const non_owning_rptr<const Asset> asset_) noexcept {
            return this_type::template hash<data_type>(this_type::project(asset_));
        }

        /**/

        template <typename Type_ = data_type, typename Options_ = multiple_options_type> requires
            std::is_trivially_copy_constructible_v<Type_> &&
            std::is_void_v<Options_>
        [[nodiscard]] static std::strong_ordering compare(
            const Type_ left_,
            const Type_ right_
        ) noexcept {
            return (index_type::look)(left_, right_);
        }

        template <typename Type_ = data_type, typename Options_ = multiple_options_type> requires
            (!std::is_trivially_copy_constructible_v<Type_>) &&
            std::is_void_v<Options_>
        [[nodiscard]] static std::strong_ordering compare(
            cref<Type_> left_,
            cref<Type_> right_
        ) noexcept {
            return (index_type::look)(left_, right_);
        }

        template <typename Type_ = data_type, typename Options_ = multiple_options_type> requires
            std::is_trivially_copy_constructible_v<Type_> &&
            (!std::is_void_v<Options_>)
        [[nodiscard]] static std::strong_ordering compare(
            const Type_ left_,
            const Type_ right_,
            cref<Options_> options_
        ) noexcept {
            return (index_type::look)(left_, right_, options_);
        }

        template <typename Type_ = data_type, typename Options_ = multiple_options_type> requires
            (!std::is_trivially_copy_constructible_v<Type_>) &&
            (!std::is_void_v<Options_>)
        [[nodiscard]] static std::strong_ordering compare(
            cref<Type_> left_,
            cref<Type_> right_,
            cref<Options_> options_
        ) noexcept {
            return (index_type::look)(left_, right_, options_);
        }
    };
}
