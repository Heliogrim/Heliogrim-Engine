#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <concepts>

namespace hg::engine::assets {
    class Asset;
}

namespace hg::engine::assets::system {
    namespace detail {
        template <typename IndexDataType_> requires std::_Is_nothrow_hashable<IndexDataType_>::value
        using projection = unary_fnc<IndexDataType_, const non_owning_rptr<const Asset>>;

        /**/

        template <typename IndexDataType_, typename MultipleOptionsType_>
        struct __lookup_comparator_selector;

        template <typename IndexDataType_>
        struct __lookup_comparator_selector<IndexDataType_, void> {
            using type = binary_fnc<std::strong_ordering, IndexDataType_, IndexDataType_>;
        };

        template <typename IndexDataType_, typename MultipleOptionsType_>
        struct __lookup_comparator_selector {
            using type = tuple_fnc<std::strong_ordering, IndexDataType_, IndexDataType_, MultipleOptionsType_>;
        };

        template <typename IndexDataType_, typename MultipleOptionsType_>
        using lookup_comparator = typename __lookup_comparator_selector<IndexDataType_, MultipleOptionsType_>::type;
    };

    template <
        bool Unique_,
        bool Multiple_,
        typename MultipleOptionsType_,
        bool Dynamic_,
        typename IndexDataType_,
        detail::projection<IndexDataType_> Projection_,
        detail::lookup_comparator<IndexDataType_, MultipleOptionsType_> LookupComparator_,
        class Relation_ = std::less<IndexDataType_>>
    struct Index {
        using this_type = Index<
            Unique_,
            Multiple_,
            MultipleOptionsType_,
            Dynamic_,
            IndexDataType_,
            Projection_,
            LookupComparator_,
            Relation_
        >;

        using unique = std::bool_constant<Unique_>;
        using multiple = std::bool_constant<Multiple_>;

        using multiple_options_type = std::conditional_t<
            std::is_void_v<MultipleOptionsType_> || std::is_empty_v<MultipleOptionsType_>,
            void,
            MultipleOptionsType_>;

        /**/

        using stored = std::bool_constant<!Dynamic_>;
        using dynamic = std::bool_constant<Dynamic_>;

        using data_type = IndexDataType_;

        using proj_type = decltype(Projection_);
        static constexpr detail::projection<data_type> proj = Projection_;

        using look_type = decltype(LookupComparator_);
        static constexpr detail::lookup_comparator<data_type, multiple_options_type> look = LookupComparator_;

        using relation_type = Relation_;
    };
}
