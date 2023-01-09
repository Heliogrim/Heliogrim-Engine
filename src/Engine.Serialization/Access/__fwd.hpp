#pragma once

#define ACCESS_LAYOUT template <typename> friend class ::ember::engine::serialization::access::Layout;
#define ACCESS_STRUCTURE template <class> friend class ::ember::engine::serialization::access::Structure;

namespace ember::engine::serialization::access {
    template <class Type_>
    class Layout;

    template <class Type_>
    class Structure;
}
