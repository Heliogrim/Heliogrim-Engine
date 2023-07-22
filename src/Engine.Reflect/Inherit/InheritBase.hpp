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
        [[nodiscard]] const __restricted_ptr<const meta_class> getMetaClass() const noexcept {
            return static_cast<const __restricted_ptr<const meta_class>>(ClassMetaBase::getMetaClass());
        }
    };
}
