#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <concepts>

namespace hg::engine::assets {
    class Asset;
}

namespace hg::engine::assets::system {
    namespace detail {
        template <typename IndexDataType_> requires _STD _Is_nothrow_hashable<IndexDataType_>::value
        using projection = unary_fnc<IndexDataType_, const non_owning_rptr<const Asset>>;

        template <typename IndexDataType_, typename MultipleOptionsType_>
        struct __comparator_selector;

        template <typename IndexDataType_>
        struct __comparator_selector<IndexDataType_, void> {
            using type = binary_fnc<_STD strong_ordering, IndexDataType_, IndexDataType_>;
        };

        template <typename IndexDataType_, typename MultipleOptionsType_>
        struct __comparator_selector {
            using type = tuple_fnc<_STD strong_ordering, IndexDataType_, IndexDataType_, MultipleOptionsType_>;
        };

        template <typename IndexDataType_, typename MultipleOptionsType_>
        using comparator = typename __comparator_selector<IndexDataType_, MultipleOptionsType_>::type;
    };

    template <
        bool Unique_,
        bool Multiple_,
        typename MultipleOptionsType_,
        bool Dynamic_,
        typename IndexDataType_,
        detail::projection<IndexDataType_> Projection_,
        detail::comparator<IndexDataType_, MultipleOptionsType_> Comparator_
    >
    struct Index {
        using this_type = Index<
            Unique_,
            Multiple_,
            MultipleOptionsType_,
            Dynamic_,
            IndexDataType_,
            Projection_,
            Comparator_
        >;

        using unique = _STD bool_constant<Unique_>;
        using multiple = _STD bool_constant<Multiple_>;

        using multiple_options_type = _STD conditional_t<
            _STD is_void_v<MultipleOptionsType_> || _STD is_empty_v<MultipleOptionsType_>,
            void,
            MultipleOptionsType_>;

        /**/

        using stored = _STD bool_constant<!Dynamic_>;
        using dynamic = _STD bool_constant<Dynamic_>;

        using data_type = IndexDataType_;

        static constexpr detail::projection<data_type> proj = Projection_;
        static constexpr detail::comparator<data_type, multiple_options_type> comp = Comparator_;
    };
}
