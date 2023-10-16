#include "Selector.hpp"

#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassComputeInvocationComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassMeshInvocationComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassSceneMeshInvocationComponent.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflect/IsType.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>

#include "PartialCompute.hpp"
#include "PartialGeneric.hpp"
#include "PartialSkeletal.hpp"
#include "PartialStatic.hpp"
#include "PartialProcedural.hpp"
#include "PartialScene.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

smr<InvocationGenerator> AccelerationInvocationSelector::operator()(
    mref<smr<graph::SubpassNode>> node_
) const noexcept {

    const auto nodeAccel = node_->getSubpassAcceleration();
    const auto nodeInvoke = node_->getSubpassInvocation();

    if (IsType<graph::SubpassComputeInvocationComponent>(*nodeInvoke)) {
        return PartialCompute {}(_STD move(node_));
    }

    if (IsType<graph::SubpassSceneMeshInvocationComponent>(*nodeInvoke)) {
        return PartialScene {}(_STD move(node_));
    }

    if (const auto smic = Cast<graph::SubpassMeshInvocationComponent>(nodeInvoke.get())) {

        const auto meshMeta = smic->getMeshDataLayout().get();
        assert(meshMeta);

        if (
            StaticMeshIG::getMeshDescription().get() == meshMeta ||
            StaticMeshIG::getMeshDescription()->isCompatible(meshMeta)
        ) {
            return PartialStatic {}(_STD move(node_));
        }

        if (
            SkeletalMeshIG::getMeshDescription().get() == meshMeta ||
            SkeletalMeshIG::getMeshDescription()->isCompatible(meshMeta)
        ) {
            return PartialSkeletal {}(_STD move(node_));
        }

        /**
         * @warning This is a special generator used with a fixed data set and predefined invocation pattern
         * @todo We need to identify this in better way... maybe we need a additional invocation component
         */
        if (StaticMeshIG::getMeshDescription().get() == meshMeta) {
            return PartialStatic {}(_STD move(node_));
        }

        // TODO: Procedural Mesh IG
        /**
         * @details This is a special generator, similar to the generic ig, but specialized to handle
         *  systems with procedural generated state
         */
        if constexpr (false) {
            return PartialProcedural {}(_STD move(node_));
        }

        /**
         * @fallthrough
         */
        return PartialGeneric {}(_STD move(node_));
    }

    return nullptr;
}
