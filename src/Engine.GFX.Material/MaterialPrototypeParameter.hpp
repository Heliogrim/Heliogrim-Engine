#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.Acc/Stage/TransferDataType.hpp>

#include "MaterialParameterStorage.hpp"

namespace hg::engine::gfx::material {
    class MaterialPrototypeParameter {
    public:
        using this_type = MaterialPrototypeParameter;

        template <typename Type_>
        using mpts_t = MaterialParameterStorage<Type_>;

    public:
        MaterialPrototypeParameter(
            mref<string> uniqueName_,
            mref<acc::TransferDataType> dataType_,
            mref<uptr<MaterialParameterStorageBase>> defaultStorage_
        ) noexcept;

        MaterialPrototypeParameter(mref<this_type> other_) noexcept = default;

        MaterialPrototypeParameter(cref<this_type>) = delete;

        ~MaterialPrototypeParameter() noexcept;

    public:
        ref<this_type> operator=(mref<this_type> other_);

        ref<this_type> operator=(cref<this_type>) = delete;

    private:
        string _uniqueName;

    public:
        [[nodiscard]] string getUniqueName() const noexcept;

    private:
        acc::TransferDataType _dataType;
        uptr<MaterialParameterStorageBase> _defaultStorage;

    public:
        [[nodiscard]] acc::TransferDataType getDataType() const noexcept;

        [[nodiscard]] nmpt<const MetaClass> getStorageClass() const noexcept;

        template <typename Type_>
        typename mpts_t<Type_>::trait_type::cref_type getDefaultValue() const {
            assert(_defaultStorage->getMetaClass()->exact<mpts_t<Type_>>());
            return static_cast<ptr<mpts_t<Type_>>>(_defaultStorage.get())->template get<Type_>();
        }

        template <typename Type_>
        void set(Type_&& value_) {
            assert(_defaultStorage->getMetaClass()->exact<mpts_t<Type_>>());
            static_cast<ptr<mpts_t<Type_>>>(_defaultStorage.get())->template set<Type_>(_STD forward<Type_>(value_));
        }
    };
}
