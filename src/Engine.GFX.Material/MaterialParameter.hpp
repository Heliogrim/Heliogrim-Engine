#pragma once

#include <cassert>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "MaterialParameterStorage.hpp"
#include "__fwd.hpp"

namespace hg::engine::gfx::material {
    class MaterialParameter {
    public:
        using this_type = MaterialParameter;

        template <typename Type_>
        using mpts_t = MaterialParameterStorage<Type_>;

    public:
        constexpr MaterialParameter() noexcept = default;

        MaterialParameter(mref<uptr<MaterialParameterStorageBase>> storage_) noexcept;

        MaterialParameter(cref<this_type>) = delete;

        MaterialParameter(mref<this_type> other_) noexcept = default;

        ~MaterialParameter();

    private:
        uptr<MaterialParameterStorageBase> _storage;

    public:
        [[nodiscard]] nmpt<const MetaClass> getStorageClass() const noexcept;

        template <typename Type_>
        typename mpts_t<Type_>::trait_type::cref_type get() const {
            assert(_storage->getMetaClass()->exact<mpts_t<Type_>>());
            return static_cast<ptr<mpts_t<Type_>>>(_storage.get())->template get<Type_>();
        }

        template <typename Type_>
        void set(Type_&& value_) {
            assert(_storage->getMetaClass()->exact<mpts_t<Type_>>());
            static_cast<ptr<mpts_t<Type_>>>(_storage.get())->template set<Type_>(std::forward<Type_>(value_));
        }

        template <typename Type_> requires std::is_trivially_copy_constructible_v<Type_>
        void set(cref<Type_> value_) {
            assert(_storage->getMetaClass()->exact<mpts_t<Type_>>());
            static_cast<ptr<mpts_t<Type_>>>(_storage.get())->template set<Type_>(value_);
        }
    };
}
