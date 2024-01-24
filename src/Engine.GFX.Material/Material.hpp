#pragma once

#include <optional>

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Meta/NoThrow.hpp>

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
        Vector<MaterialParameter> _parameters;

    public:
        [[nodiscard]] cref<Vector<MaterialParameter>> getParameters() const noexcept;

        [[nodiscard]] bool hasParam(string_view name_) const noexcept;

        template <typename Type_>
        bool setParam(string_view name_, Type_&& value_);

        template <typename Type_>
        std::optional<typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type> getParam(
            ParameterIdentifier identifier_,
            nothrow_t
        ) const noexcept;

        template <typename Type_>
        typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type getParam(
            ParameterIdentifier identifier_
        ) const;

        template <typename Type_>
        std::optional<typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type> getParam(
            string_view name_,
            nothrow_t
        ) const noexcept;

        template <typename Type_>
        typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type getParam(string_view name_) const;

    public:
        [[nodiscard]] Material clone() const noexcept;
    };

    /**/

    template <typename Type_>
    bool Material::setParam(string_view name_, Type_&& value_) {

        const auto& mpp = _prototype->getParameters();
        const auto iter = std::ranges::find(
            mpp,
            name_,
            [](cref<MaterialPrototypeParameter> param_) {
                return param_.getName();
            }
        );

        if (iter == mpp.end()) {
            return false;
        }

        const auto diff = std::distance(mpp.begin(), iter);
        const auto it = _parameters.begin() + diff;

        if (it->getStorageClass()->template isExactType<MaterialParameterStorage<Type_>>()) {
            it->template set<Type_>(std::forward<Type_>(value_));
            return true;
        }

        return false;
    }

    template <typename Type_>
    std::optional<typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type> Material::getParam(
        ParameterIdentifier identifier_,
        nothrow_t
    ) const noexcept {

        using result_type = std::optional<typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type>;

        if (identifier_.data >= _parameters.size()) {
            return result_type {};
        }

        const auto it = _parameters.begin() + identifier_.data;
        return result_type { it->get<Type_>() };
    }

    template <typename Type_>
    typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type Material::getParam(
        ParameterIdentifier identifier_
    ) const {
        return (_parameters.begin() + identifier_.data)->get<Type_>();
    }

    template <typename Type_>
    std::optional<typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type> Material::getParam(
        string_view name_,
        nothrow_t
    ) const noexcept {

        using result_type = std::optional<typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type>;

        const auto& mpp = _prototype->getParameters();
        const auto iter = std::ranges::find(
            mpp,
            name_,
            [](cref<MaterialPrototypeParameter> param_) {
                return param_.getName();
            }
        );

        if (iter == mpp.end()) {
            return result_type {};
        }

        const auto diff = std::distance(mpp.begin(), iter);
        const auto it = _parameters.begin() + diff;

        return result_type { it->template get<Type_>() };
    }

    template <typename Type_>
    typename MaterialParameter::mpts_t<Type_>::trait_type::cref_type Material::getParam(string_view name_) const {

        const auto& mpp = _prototype->getParameters();
        const auto iter = std::ranges::find(
            mpp,
            name_,
            [](cref<MaterialPrototypeParameter> param_) {
                return param_.getName();
            }
        );

        const auto diff = std::distance(mpp.begin(), iter);
        const auto it = _parameters.begin() + diff;

        return it->template get<Type_>();
    }
}
