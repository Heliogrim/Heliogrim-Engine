#pragma once
#include "ComputePassSpecification.hpp"
#include "GraphicsPassSpecification.hpp"
#include "MeshPassSpecification.hpp"
#include "RaytracingPassSpecification.hpp"

namespace hg::engine::gfx::acc {
    class __declspec(novtable) EffectSpecification {
    public:
        using this_type = EffectSpecification;

    protected:
        EffectSpecification() noexcept = default;

    public:
        EffectSpecification(cref<this_type>) = delete;

        EffectSpecification(mref<this_type>) = delete;

        virtual ~EffectSpecification() = default;

    public:
        /**
         * @details The targeted symbols to export. Primarly used for optimization
         *  and trimming of the provided effect.
         */
        [[nodiscard]] virtual cref<Vector<smr<const Symbol>>> targetSymbols() const noexcept = 0;

    public:
        template <typename Type_>
        [[nodiscard]] Type_ getPassSpec() const;

    public:
        [[nodiscard]] virtual ComputePassSpecification getComputeSpec() const = 0;

        [[nodiscard]] virtual GraphicsPassSpecification getGraphicsSpec() const = 0;

        [[nodiscard]] virtual MeshPassSpecification getMeshSpec() const = 0;

        [[nodiscard]] virtual RaytracingPassSpecification getRaytracingSpec() const = 0;
    };

    template <>
    inline GraphicsPassSpecification EffectSpecification::getPassSpec<GraphicsPassSpecification>() const {
        return this->getGraphicsSpec();
    }

    template <>
    inline MeshPassSpecification EffectSpecification::getPassSpec<MeshPassSpecification>() const {
        return this->getMeshSpec();
    }

    template <>
    inline RaytracingPassSpecification EffectSpecification::getPassSpec<RaytracingPassSpecification>() const {
        return this->getRaytracingSpec();
    }

    template <>
    inline ComputePassSpecification EffectSpecification::getPassSpec<ComputePassSpecification>() const {
        return this->getComputeSpec();
    }
}
