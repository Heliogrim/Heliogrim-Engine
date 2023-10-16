#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "EffectSpecification.hpp"

namespace hg::engine::gfx::acc {
    class SimpleEffectSpecification :
        public EffectSpecification {
    public:
        using this_type = SimpleEffectSpecification;

    public:
        SimpleEffectSpecification() noexcept = default;

        SimpleEffectSpecification(mref<Vector<smr<const Symbol>>> targetSymbols_) noexcept;

        ~SimpleEffectSpecification() noexcept override = default;

    private:
        Vector<smr<const Symbol>> _targetSymbols;

    public:
        [[nodiscard]] cref<Vector<smr<const Symbol>>> targetSymbols() const noexcept override;

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
        void setPassSpec(mref<uptr<Type_>> spec_);
    };

    template <>
    inline void SimpleEffectSpecification::setPassSpec<ComputePassSpecification>(
        mref<uptr<ComputePassSpecification>> spec_
    ) {
        _cps = _STD move(spec_);
    }

    template <>
    inline void SimpleEffectSpecification::setPassSpec<GraphicsPassSpecification>(
        mref<uptr<GraphicsPassSpecification>> spec_
    ) {
        _gps = _STD move(spec_);
    }

    template <>
    inline void SimpleEffectSpecification::setPassSpec<MeshPassSpecification>(
        mref<uptr<MeshPassSpecification>> spec_
    ) {
        _mps = _STD move(spec_);
    }

    template <>
    inline void SimpleEffectSpecification::setPassSpec<RaytracingPassSpecification>(
        mref<uptr<RaytracingPassSpecification>> spec_
    ) {
        _rps = _STD move(spec_);
    }
}
