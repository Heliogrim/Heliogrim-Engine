#pragma once
#include "InheritMeta.hpp"

namespace hg {
    template <typename Derived_>
    class InheritBase :
        public ClassMetaBase {
    public:
        using this_type = InheritBase<Derived_>;

        using __inherit_types = reflect::__type_list<ClassMetaBase>;
        using meta_class = TypedMetaClass<Derived_, __inherit_types>;

        constexpr static auto __inherit_gen = meta_class::get();

    public:
        InheritBase() :
            ClassMetaBase() {
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
