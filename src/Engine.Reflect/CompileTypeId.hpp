#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

#include "CompileString.hpp"

namespace hg {
    namespace {
        template <typename Type_>
        constexpr CompileString compile_type_name() {
            return { __FUNCSIG__ };
        }
    }

    template <typename Type_>
    constexpr type_id ctid() {
        return type_id { compile_type_name<Type_>().hash() };
    }
}
