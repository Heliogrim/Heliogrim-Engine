#pragma once
#include <Engine.Common/Wrapper.hpp>

namespace hg {
    class __declspec(novtable) ClassMetaBase {
    public:
        template <typename>
        friend class InheritBase;

        template <typename, typename, typename...>
        friend class InheritMeta;

        template <typename>
        friend struct lookup;

    public:
        using this_type = ClassMetaBase;

    public:
        ClassMetaBase() noexcept = default;

    private:
        __restricted_ptr<const class MetaClass> _meta = nullptr;

    public:
        [[nodiscard]] const __restricted_ptr<const class MetaClass> getMetaClass() const noexcept {
            return _meta;
        }
    };
}
