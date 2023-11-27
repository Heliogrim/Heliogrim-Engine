#pragma once

#include <Engine.Common/Wrapper.hpp>
#include "Inherit/Concept.hpp"
#include "Meta/MetaClass.hpp"

namespace hg {
    template <typename Type_>
    struct ExactTypeFn;

    template <typename Type_> requires (ClassHasMeta<Type_>)
    struct ExactTypeFn<Type_> {
        template <typename Arg_> requires ClassHasMeta<_STD remove_cvref_t<Arg_>>
        [[nodiscard]] constexpr bool operator()(Arg_&& arg_) const noexcept {
            return arg_.getMetaClass()->template exact<Type_>();
        }
    };

    template <typename Type_> requires (not ClassHasMeta<Type_>)
    struct ExactTypeFn<Type_> {
        static_assert(not ClassHasMeta<Type_>);
    };

    template <ClassHasMeta Type_>
    static constexpr ExactTypeFn<Type_> ExactType {};
}
