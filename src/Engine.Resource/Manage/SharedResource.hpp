#pragma once
#include "Resource.hpp"

namespace ember::engine::resource {
    template <typename ManagedType_>
    class __declspec(novtable) SharedResource :
        public Resource<ManagedType_> {
    public:
        using this_type = SharedResource<ManagedType_>;
        using underlying_type = Resource<ManagedType_>;

        using underlying_type::value_type;

    protected:
        SharedResource() = default;
    };
}
