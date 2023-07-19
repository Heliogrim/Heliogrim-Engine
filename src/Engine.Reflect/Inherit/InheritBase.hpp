#pragma once
#include "InheritMeta.hpp"

namespace hg {
    template <typename Derived_>
    class InheritBase :
        public ClassMetaBase {
    public:
        using this_type = InheritBase<Derived_>;

    public:
        InheritBase() :
            ClassMetaBase() {
            ClassMetaBase::_meta = TypedMetaClass<Derived_>::getOrCreate();
        }

    public:
        [[nodiscard]] const __restricted_ptr<const class TypedMetaClass<Derived_>> getMetaClass() const noexcept {
            return static_cast<const __restricted_ptr<const class TypedMetaClass<Derived_>>>(
                ClassMetaBase::getMetaClass());
        }
    };
}
