#pragma once
#include "AccelerationPassFactory.hpp"

namespace hg::engine::accel {
    class VkAccelerationPassFactory final :
        public AccelerationPassFactory {
    public:
        using this_type = VkAccelerationPassFactory;

    public:
        constexpr VkAccelerationPassFactory() noexcept = default;

        constexpr ~VkAccelerationPassFactory() noexcept override = default;

    public:
        [[nodiscard]] tl::expected<smr<const GraphicsPass>, AccelerationPassFactoryError> buildGraphicsPass(
            mref<smr<const GraphicsPipeline>> pipeline_
        ) const noexcept override;

        [[nodiscard]] tl::expected<smr<const GraphicsPass>, AccelerationPassFactoryError> buildGraphicsPass(
            cref<Vector<smr<const render::graph::Symbol>>> outputSymbols_,
            cref<Vector<smr<const render::graph::Symbol>>> inputSymbols_
        ) const noexcept override;

        [[nodiscard]] tl::expected<smr<const GraphicsPass>, AccelerationPassFactoryError> buildGraphicsPass(
            cref<Vector<smr<const render::graph::Symbol>>> outputSymbols_,
            cref<Vector<smr<const render::graph::Symbol>>> inputSymbols_,
            mref<Vector<vk::SubpassDependency>> subpassDependencies_,
            u32 viewMask_,
            u32 correlationMask_
        ) const noexcept override;

        [[nodiscard]] tl::expected<smr<const RaytracingPass>, AccelerationPassFactoryError> buildRaytracingPass(
            mref<smr<const RaytracingPipeline>> pipeline_
        ) const noexcept override;

        [[nodiscard]] tl::expected<smr<const RaytracingPass>, AccelerationPassFactoryError> buildRaytracingPass(
            cref<Vector<smr<const render::graph::Symbol>>> outputSymbols_,
            cref<Vector<smr<const render::graph::Symbol>>> inputSymbols_
        ) const noexcept override;

        [[nodiscard]] tl::expected<smr<const MeshPass>, AccelerationPassFactoryError> buildMeshPass(
            mref<smr<const MeshPipeline>> pipeline_
        ) const noexcept override;

        [[nodiscard]] tl::expected<smr<const MeshPass>, AccelerationPassFactoryError> buildMeshPass(
            cref<Vector<smr<const render::graph::Symbol>>> outputSymbols_,
            cref<Vector<smr<const render::graph::Symbol>>> inputSymbols_
        ) const noexcept override;
    };
}
