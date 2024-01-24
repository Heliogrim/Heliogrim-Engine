#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Reflect/Inherit/ClassMetaBase.hpp>

#include <cassert>
#include <Engine.Reflect/Inherit/Concept.hpp>

namespace hg::engine::serialization {
    class DataLayoutReflect {
    public:
        using this_type = DataLayoutReflect;

    public:
        DataLayoutReflect();

        DataLayoutReflect(cref<this_type> other_) noexcept = default;

        DataLayoutReflect(mref<this_type> other_) noexcept = default;

        ~DataLayoutReflect() noexcept;

    private:
        unary_fnc<ptr<void>, const ptr<void>> _ipc;
        unary_fnc<void, const ptr<void>> _ipd;

        // TODO: check whether we want to store more helper functions like `indirect resize` or `indirect shrink_to_fit` for container optimization
        // binary_fnc<void, const ptr<void>, const u64> _pre; // `self->resize(const u64 count_)`
        // unary_fnc<void, const ptr<void>> _post; // `self->shrink_to_fit()` | `self->clear()`

    public:
        [[nodiscard]] ptr<ClassMetaBase> instantiate() const;

        [[deprecated]] void destroy(mref<ptr<ClassMetaBase>> obj_) const;

        void constructInPlace(const ptr<void> dst_) const;

        void destroyInPlace(const ptr<void> dst_) const;

    private:
        ptr<const MetaClass> _rclass;

    public:
        [[nodiscard]] bool hasClass() const noexcept;

        [[nodiscard]] non_owning_rptr<const MetaClass> getClass() const noexcept;

    private:
        template <typename Type_>
        static ptr<void> sipch(const ptr<void> dst_) {
            return new(dst_) Type_;
        }

        template <typename Type_>
        static void sipdh(const ptr<void> dst_) {
            static_cast<const ptr<Type_>>(dst_)->~Type_();
        }

    public:
        template <typename SubjectType_> requires std::is_default_constructible_v<SubjectType_>
        void storeType() {

            if constexpr (ClassHasMeta<SubjectType_>) {
                _rclass = TypedMetaClass<SubjectType_>::get();
                assert(_rclass);

            } else {

                this_type::_ipc = this_type::sipch<SubjectType_>;
                this_type::_ipd = this_type::sipdh<SubjectType_>;
            }

            // TODO: optimization functions

        }
    };
}
