#include "Selector.hpp"

#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassMaterialAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassMultiAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassSingleAccelComponent.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>

#include "PartialMaterial.hpp"
#include "PartialMulti.hpp"
#include "PartialSingle.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

smr<SubpassStage> AccelerationPassSelector::operator()(
    mref<smr<graph::SubpassNode>> node_,
    mref<uptr<InvocationGenerator>> ig_
) const noexcept {
    return switchType(
        node_->getSubpassAcceleration().get(),
        [&node_, &ig_](graph::SubpassSingleAccelComponent*) {
            return PartialSingle {}(_STD move(node_), _STD move(ig_)).into<SubpassStage>();
        },
        [&node_, &ig_](graph::SubpassMultiAccelComponent*) {
            return PartialMulti {}(_STD move(node_), _STD move(ig_)).into<SubpassStage>();
        },
        [&node_, &ig_](graph::SubpassMaterialAccelComponent*) {
            return PartialMaterial {}(_STD move(node_), _STD move(ig_)).into<SubpassStage>();
        }
    );

}
