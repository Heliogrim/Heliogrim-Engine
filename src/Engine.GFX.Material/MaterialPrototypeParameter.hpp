#pragma once

#include <Engine.Accel.Effect/Stage/TransferDataType.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "MaterialParameterStorage.hpp"
#include "ParameterIdentifier.hpp"

namespace hg::engine::gfx::material {
    class MaterialPrototypeParameter {
    public:
        using this_type = MaterialPrototypeParameter;

        template <typename Type_>
        using mpts_t = MaterialParameterStorage<Type_>;

    public:
        MaterialPrototypeParameter(
            mref<ParameterIdentifier> identifier_,
            mref<String> name_,
            mref<accel::TransferDataType> dataType_,
            mref<uptr<MaterialParameterStorageBase>> defaultStorage_
        ) noexcept;

        MaterialPrototypeParameter(mref<this_type> other_) noexcept = default;

        MaterialPrototypeParameter(cref<this_type>) = delete;

        ~MaterialPrototypeParameter() noexcept;

    public:
        ref<this_type> operator=(mref<this_type> other_) noexcept;

        ref<this_type> operator=(cref<this_type>) = delete;

    private:
        ParameterIdentifier _identifier;
        String _name;

    public:
        [[nodiscard]] ParameterIdentifier getId() const noexcept;

        [[nodiscard]] StringView getName() const noexcept;

    private:
        accel::TransferDataType _dataType;
        uptr<MaterialParameterStorageBase> _defaultStorage;

    public:
        [[nodiscard]] accel::TransferDataType getDataType() const noexcept;

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
