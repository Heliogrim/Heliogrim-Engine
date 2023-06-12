#pragma once

#include <optional>

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Meta/NoThrow.hpp>
#include <Engine.GFX.Acc/__fwd.hpp>

#include "__fwd.hpp"
#include "MaterialParameter.hpp"
#include "MaterialPrototype.hpp"
#include "MaterialPrototypeParameter.hpp"

namespace hg::engine::gfx::material {
    class Material {
    public:
        using this_type = Material;

    public:
        Material() noexcept;

        Material(
            mref<Guid> guid_,
            mref<smr<MaterialPrototype>> prototype_,
            mref<Set<smr<acc::AccelerationPass>>> passes_,
            mref<Vector<MaterialParameter>> parameters_
        ) noexcept;

        ~Material();

    private:
        Guid _guid;

    public:
        [[nodiscard]] Guid getGuid() const noexcept;

    private:
        smr<MaterialPrototype> _prototype;

    public:
        [[nodiscard]] smr<MaterialPrototype> getPrototype() const noexcept;

    private:
        Set<smr<acc::AccelerationPass>> _accelerationPasses;

    public:
        [[nodiscard]] cref<Set<smr<acc::AccelerationPass>>> getAccelerationPasses() const noexcept;

    private:
        Vector<MaterialParameter> _parameters;

    public:
        [[nodiscard]] cref<Vector<MaterialParameter>> getParameters() const noexcept;

        [[nodiscard]] bool hasParam(string_view uniqueName_) const noexcept;

        template <typename Type_>
        bool setParam(string_view uniqueName_, Type_&& value_);

        template <typename Type_>
        _STD optional<typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type> getParam(
            string_view uniqueName_,
            nothrow_t
        ) const noexcept;

        template <typename Type_>
        typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type getParam(string_view uniqueName_) const;

    public:
        [[nodiscard]] Material clone() const noexcept;
    };

    /**/

    template <typename Type_>
    bool Material::setParam(string_view uniqueName_, Type_&& value_) {

        const auto& mpp = _prototype->getParameters();
        const auto iter = _STD ranges::find(
            mpp,
            uniqueName_,
            [](cref<MaterialPrototypeParameter> param_) {
                return param_.getUniqueName();
            }
        );

        if (iter == mpp.end()) {
            return false;
        }

        const auto diff = _STD distance(mpp.begin(), iter);
        const auto it = _parameters.begin() + diff;

        if (it->getStorageClass()->template isExactType<MaterialParameterStorage<Type_>>()) {
            it->template set<Type_>(_STD forward<Type_>(value_));
            return true;
        }

        return false;
    }

    template <typename Type_>
    _STD optional<typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type> Material::getParam(
        string_view uniqueName_,
        nothrow_t
    ) const noexcept {

        using result_type = _STD optional<typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type>;

        const auto& mpp = _prototype->getParameters();
        const auto iter = _STD ranges::find(
            mpp,
            uniqueName_,
            [](cref<MaterialPrototypeParameter> param_) {
                return param_.getUniqueName();
            }
        );

        if (iter == mpp.end()) {
            return result_type {};
        }

        const auto diff = _STD distance(mpp.begin(), iter);
        const auto it = _parameters.begin() + diff;

        return result_type { it->template get<Type_>() };
    }

    template <typename Type_>
    typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type Material::getParam(string_view uniqueName_) const {

        const auto& mpp = _prototype->getParameters();
        const auto iter = _STD ranges::find(
            mpp,
            uniqueName_,
            [](cref<MaterialPrototypeParameter> param_) {
                return param_.getUniqueName();
            }
        );

        const auto diff = _STD distance(mpp.begin(), iter);
        const auto it = _parameters.begin() + diff;

        return it->template get<Type_>();
    }
}
