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

        using __inherit_types = typename reflect::__type_list_append<
            reflect::__type_list<Base_, Rest_...>,
            typename reflect::__type_list_aggregate<Base_, Rest_...>::type
        >::type;
        using meta_class = TypedMetaClass<Derived_, __inherit_types>;

        constexpr static auto __inherit_gen = meta_class::get();

    public:
        InheritMeta() :
            Base_(),
            Rest_()... {
            ClassMetaBase::_meta = __inherit_gen;
        }

        template <typename... Args_>
        InheritMeta(Args_&&... args_) :
            Base_(_STD forward<Args_>(args_)...),
            Rest_()... {
            ClassMetaBase::_meta = __inherit_gen;
        }

    public:
        /**
         * Will query for the dynamically stored reference to the related meta object.
         *
         * @warning You shall be strictly prevent writing access to the returned data.
         * @returns A pointer to the compile-time object.
         */
        [[nodiscard]] const __restricted_ptr<const meta_class> getMetaClass() const noexcept {
            return static_cast<const __restricted_ptr<const meta_class>>(ClassMetaBase::getMetaClass());
        }

        /**
         * Will query statically for the compile-time instantiated meta class object related to this type.
         *
         * @warning You shall be strictly prevent writing access to the returned data.
         * @returns A pointer to the compile-time object.
         */
        [[nodiscard]] constexpr static const __restricted_ptr<const meta_class> getStaticMetaClass() noexcept {
            return meta_class::get();
        }
    };
}
