#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "SpecificationStorage.hpp"

namespace hg::engine::gfx::acc {
    class SimpleSpecificationStorage :
        public SpecificationStorage {
    public:
        using this_type = SimpleSpecificationStorage;

    public:
        SimpleSpecificationStorage() noexcept = default;

        ~SimpleSpecificationStorage() noexcept override = default;

    private:
        uptr<ComputePassSpecification> _cps;
        uptr<GraphicsPassSpecification> _gps;
        uptr<MeshPassSpecification> _mps;
        uptr<RaytracingPassSpecification> _rps;

    public:
        [[nodiscard]] ComputePassSpecification getComputeSpec() const override;

        [[nodiscard]] GraphicsPassSpecification getGraphicsSpec() const override;

        [[nodiscard]] MeshPassSpecification getMeshSpec() const override;

        [[nodiscard]] RaytracingPassSpecification getRaytracingSpec() const override;

    public:
        template <typename Type_>
        void set(mref<uptr<Type_>> spec_);
    };

    template <>
    inline void SimpleSpecificationStorage::set<ComputePassSpecification>(
        mref<uptr<ComputePassSpecification>> spec_
    ) {
        _cps = _STD move(spec_);
    }

    template <>
    inline void SimpleSpecificationStorage::set<GraphicsPassSpecification>(
        mref<uptr<GraphicsPassSpecification>> spec_
    ) {
        _gps = _STD move(spec_);
    }

    template <>
    inline void SimpleSpecificationStorage::set<MeshPassSpecification>(
        mref<uptr<MeshPassSpecification>> spec_
    ) {
        _mps = _STD move(spec_);
    }

    template <>
    inline void SimpleSpecificationStorage::set<RaytracingPassSpecification>(
        mref<uptr<RaytracingPassSpecification>> spec_
    ) {
        _rps = _STD move(spec_);
    }
}
