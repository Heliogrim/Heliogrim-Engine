#pragma once
#include "Concept.hpp"

namespace hg::reflect {
    template <typename...>
    struct __type_list;

    /**/

    template <typename, bool>
    struct __type_list_lookup_impl;

    template <typename Type_>
    struct __type_list_lookup_impl<Type_, false> {
        using type = __type_list<>;
    };

    template <typename Type_>
    struct __type_list_lookup_impl<Type_, true> {
        using type = typename Type_::__inherit_types;
    };

    template <typename Type_>
    using __type_list_lookup = typename __type_list_lookup_impl<Type_, __HasInheritTypeList<Type_>>::type;

    /**/

    template <typename, typename>
    struct __type_list_append;

    template <typename... Ts_, typename... Us_>
    struct __type_list_append<__type_list<Ts_...>, __type_list<Us_...>> {
        using type = __type_list<Ts_..., Us_...>;
    };

    /**/

    template <typename...>
    struct __type_list_aggregate;

    template <typename Last_>
    struct __type_list_aggregate<Last_> {
        using type = __type_list_lookup<Last_>;
    };

    template <typename Next_, typename... Rest_>
    struct __type_list_aggregate<Next_, Rest_...> {
        using type = typename __type_list_append<
            __type_list_lookup<Next_>,
            typename __type_list_aggregate<Rest_...>::type
        >::type;
    };
}
