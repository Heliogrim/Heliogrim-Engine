#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>

#include "IGCommandBuffer.hpp"
#include "InvocationGenerator.hpp"

namespace hg::engine::gfx::render::pipeline {
    class StaticMeshIG :
        public InvocationGenerator {
    public:
        StaticMeshIG() noexcept;

        ~StaticMeshIG() noexcept override;

    public:
        [[nodiscard]] static smr<graph::MeshDescription> getMeshDescription();

    public:
        void operator()();

        [[nodiscard]] IGProcessResult operator()(
            mref<smr<const acc::AccelerationPass>> acceleration_,
            mref<uptr<IGCommandBuffer>> igcb_
        ) override;

        tl::expected<AccelerationCommandBuffer, IGError> finalize() noexcept override;
    };
}
