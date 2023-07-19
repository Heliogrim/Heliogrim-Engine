#pragma once

#include <concepts>
#include <Engine.Common/Wrapper.hpp>

namespace hg {
    class ClassMetaBase;

    template <class TargetType_>
    class TypedMetaClass;
}

namespace hg {
    namespace {
        template <class Type_>
        concept __DirectMetaInherit = requires(const Type_& obj_) {
            { obj_.getMetaClass() } -> _STD same_as<const ptr<const TypedMetaClass<Type_>>>;
        };

        template <
            class Type_,
            class RetTy_ = std::invoke_result_t<decltype(&Type_::getMetaClass)(Type_)>>
        concept __IndirectMetaInhert = _STD derived_from<Type_, typename RetTy_::target_type>;
    }

    template <class Type_>
    concept ClassHasMeta = (__DirectMetaInherit<Type_> || __IndirectMetaInhert<Type_>)
        && _STD derived_from<Type_, ClassMetaBase>;
}
