#pragma once

#include "ClassMetaBase.hpp"
#include "../Meta/TypedMetaClass.hpp"

namespace hg {
    /**
     * @warning Be aware that polymorphic inheritance is fine; Just don't use diamond inheritance !?!
     */
    template <typename Derived_, typename Base_, typename... Rest_>
    class InheritMeta :
        public Base_,
        public Rest_... {
    public:
        using this_type = InheritMeta<Derived_, Base_, Rest_...>;

    public:
        InheritMeta() :
            Base_(),
            Rest_()... {
            ClassMetaBase::_meta = TypedMetaClass<Derived_>::template inherit<Base_, Rest_...>();
        }

        template <typename... Args_>
        InheritMeta(Args_&&... args_) :
            Base_(_STD forward<Args_>(args_)...),
            Rest_()... {
            ClassMetaBase::_meta = TypedMetaClass<Derived_>::template inherit<Base_, Rest_...>();
        }

    public:
        [[nodiscard]] const __restricted_ptr<const class TypedMetaClass<Derived_>> getMetaClass() const noexcept {
            return static_cast<const __restricted_ptr<const class TypedMetaClass<Derived_>>>(
                ClassMetaBase::getMetaClass());
        }
    };
}
