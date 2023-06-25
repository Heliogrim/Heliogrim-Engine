#pragma once
#include "ComputePassSpecification.hpp"
#include "GraphicsPassSpecification.hpp"
#include "MeshPassSpecification.hpp"
#include "RaytracingPassSpecification.hpp"

namespace hg::engine::gfx::acc {
    class SpecificationStorage {
    public:
        using this_type = SpecificationStorage;

    protected:
        SpecificationStorage() noexcept = default;

    public:
        SpecificationStorage(cref<this_type>) = delete;

        SpecificationStorage(mref<this_type>) = delete;

        virtual ~SpecificationStorage() = default;

    public:
        template <typename Type_>
        [[nodiscard]] Type_ get() const;

    public:
        [[nodiscard]] virtual ComputePassSpecification getComputeSpec() const = 0;

        [[nodiscard]] virtual GraphicsPassSpecification getGraphicsSpec() const = 0;

        [[nodiscard]] virtual MeshPassSpecification getMeshSpec() const = 0;

        [[nodiscard]] virtual RaytracingPassSpecification getRaytracingSpec() const = 0;
    };

    template <>
    inline GraphicsPassSpecification SpecificationStorage::get<GraphicsPassSpecification>() const {
        return this->getGraphicsSpec();
    }

    template <>
    inline MeshPassSpecification SpecificationStorage::get<MeshPassSpecification>() const {
        return this->getMeshSpec();
    }

    template <>
    inline RaytracingPassSpecification SpecificationStorage::get<RaytracingPassSpecification>() const {
        return this->getRaytracingSpec();
    }

    template <>
    inline ComputePassSpecification SpecificationStorage::get<ComputePassSpecification>() const {
        return this->getComputeSpec();
    }
}
