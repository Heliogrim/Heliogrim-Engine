#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <tl/expected.hpp>

#include "GraphicsPass.hpp"
#include "MeshPass.hpp"
#include "RaytracingPass.hpp"
#include "../Pipeline/GraphicsPipeline.hpp"
#include "Engine.GFX.Acc/Pipeline/MeshPipeline.hpp"
#include "Engine.GFX.Acc/Pipeline/RaytracingPipeline.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationPassFactoryError final : public _STD exception {};

    class __declspec(novtable) AccelerationPassFactory {
    public:
        using this_type = AccelerationPassFactory;

    public:
        constexpr AccelerationPassFactory() noexcept = default;

        constexpr virtual ~AccelerationPassFactory() noexcept = default;

    public:
        [[nodiscard]] virtual tl::expected<smr<const GraphicsPass>, AccelerationPassFactoryError> buildGraphicsPass(
            mref<smr<const GraphicsPipeline>> pipeline_
        ) const noexcept = 0;

        [[nodiscard]] virtual tl::expected<smr<const GraphicsPass>, AccelerationPassFactoryError> buildGraphicsPass(
            cref<Vector<smr<const Symbol>>> outputSymbols_,
            cref<Vector<smr<const Symbol>>> inputSymbols_ = {}
        ) const noexcept = 0;

        [[nodiscard]] virtual tl::expected<smr<const RaytracingPass>, AccelerationPassFactoryError> buildRaytracingPass(
            mref<smr<const RaytracingPipeline>> pipeline_
        ) const noexcept = 0;

        [[nodiscard]] virtual tl::expected<smr<const RaytracingPass>, AccelerationPassFactoryError> buildRaytracingPass(
            cref<Vector<smr<const Symbol>>> outputSymbols_,
            cref<Vector<smr<const Symbol>>> inputSymbols_ = {}
        ) const noexcept = 0;

        [[nodiscard]] virtual tl::expected<smr<const MeshPass>, AccelerationPassFactoryError> buildMeshPass(
            mref<smr<const MeshPipeline>> pipeline_
        ) const noexcept = 0;

        [[nodiscard]] virtual tl::expected<smr<const MeshPass>, AccelerationPassFactoryError> buildMeshPass(
            cref<Vector<smr<const Symbol>>> outputSymbols_,
            cref<Vector<smr<const Symbol>>> inputSymbols_ = {}
        ) const noexcept = 0;
    };
}
