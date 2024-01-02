#include "VkRCmdTranslator.hpp"

#include <Engine.Accel.Pass/VkGraphicsPass.hpp>
#include <Engine.Accel.Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.GFX.Render.Command/RenderCommandIterator.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Render.Command/Commands/AttachResource.hpp>
#include <Engine.GFX.Render.Command/Commands/BeginAccelerationPass.hpp>
#include <Engine.GFX.Render.Command/Commands/BindIndexBuffer.hpp>
#include <Engine.GFX.Render.Command/Commands/BindPipeline.hpp>
#include <Engine.GFX.Render.Command/Commands/BindStorageBuffer.hpp>
#include <Engine.GFX.Render.Command/Commands/BindTexture.hpp>
#include <Engine.GFX.Render.Command/Commands/BindUniformBuffer.hpp>
#include <Engine.GFX.Render.Command/Commands/BindVertexBuffer.hpp>
#include <Engine.GFX.Render.Command/Commands/DrawDispatch.hpp>
#include <Engine.GFX.Render.Command/Commands/DrawDispatchIndirect.hpp>
#include <Engine.GFX.Render.Command/Commands/DrawMesh.hpp>
#include <Engine.GFX.Render.Command/Commands/Lambda.hpp>
#include <Engine.GFX/Buffer/StorageBufferView.hpp>
#include <Engine.GFX/Buffer/UniformBufferView.hpp>
#include <Engine.GFX/Texture/SampledTextureView.hpp>
#include <Engine.GFX/Texture/TextureView.hpp>
#include <Engine.GFX/Texture/VirtualTextureView.hpp>
#include <Engine.Reflect/Cast.hpp>

#include "VkResourceTable.hpp"

using namespace hg::driver::vk;
using namespace hg::engine::render;
using namespace hg::engine::accel;
using namespace hg;

uptr<cmd::NativeBatch> VkRCmdTranslator::operator()(const ptr<const cmd::RenderCommandBuffer> commands_) noexcept {

    assert(commands_->root());

    /**/
    auto batch = make_uptr<VkNativeBatch>();

    auto device = engine::Engine::getEngine()->getGraphics()->getCurrentDevice();
    auto ecpool = device->graphicsQueue()->pool();

    AccelCommandBuffer eccmd = ecpool->make();
    VkResourceTable rt {};

    /**/

    auto* state = new VkState(VkScopedCmdMgr { 0u, eccmd });
    auto iter = cmd::RenderCommandIterator { commands_->root().get() };

    while (iter.valid()) {
        (*iter++)(state, this);
    }

    rt.store(_STD move(state->srt));
    delete state;

    /**/
    batch->add(make_uptr<AccelCommandBuffer>(_STD move(eccmd)));
    batch->add(make_uptr<VkResourceTable>(_STD move(rt)));
    /**/

    return batch;
}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::RenderCommand>) noexcept {}

void VkRCmdTranslator::translate(const ptr<State> state_, ptr<const cmd::BeginRCmd> cmd_) noexcept {

    auto active = static_cast<VkState*>(state_)->cmd.active();
    active->begin(nullptr);
}

void VkRCmdTranslator::translate(
    const ptr<State> state_,
    const ptr<const cmd::BeginAccelerationPassRCmd> cmd_
) noexcept {

    auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    const auto graphicsPass = Cast<VkGraphicsPass>(cmd_->data.pass.get());
    assert(graphicsPass);

    active->bindRenderPass(
        {
            .graphicsPass = *graphicsPass,
            .framebuffer = cmd_->data.framebuffer.load(),
            .subrecording = false,
            .clearValues = cmd_->data.clearValues
        }
    );
}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::BeginSubPassRCmd> cmd_) noexcept {}

void VkRCmdTranslator::translate(const ptr<State> state_, ptr<const cmd::NextSubpassRCmd> cmd_) noexcept {

    auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    active->vkCommandBuffer().nextSubpass(::vk::SubpassContents::eInline);
}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::EndSubPassRCmd> cmd_) noexcept {}

void VkRCmdTranslator::translate(const ptr<State> state_, ptr<const cmd::EndAccelerationPassRCmd> cmd_) noexcept {

    auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    active->endRenderPass();
}

void VkRCmdTranslator::translate(const ptr<State> state_, ptr<const cmd::EndRCmd> cmd_) noexcept {

    auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    active->end();
}

void VkRCmdTranslator::translate(const ptr<State> state_, const ptr<const cmd::BindPipelineRCmd> cmd_) noexcept {

    auto* const state = static_cast<VkState*>(state_);

    auto active = state->cmd.active();
    assert(active.has_value());

    // TODO: assert(active->getFeatureSet() & cmd_->featureSet);

    const auto graphicsPipeline = Cast<VkGraphicsPipeline>(cmd_->pipeline.get());
    assert(graphicsPipeline);

    state->srt.replaceActivePipeline(graphicsPipeline);
    active->bindPipeline(graphicsPipeline);
}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::BindResourceTableRCmd> cmd_) noexcept {}

void VkRCmdTranslator::translate(const ptr<State> state_, const ptr<const cmd::BindIndexBufferRCmd> cmd_) noexcept {

    auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    const auto& indexView = cmd_->_indexBufferView;
    active->bindIndexBuffer(indexView);
}

void VkRCmdTranslator::translate(const ptr<State> state_, const ptr<const cmd::BindVertexBufferRCmd> cmd_) noexcept {

    auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    const auto& vertexView = cmd_->_vertexBufferView;
    active->bindVertexBuffer(0uL, vertexView);
}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::BindSkeletalMeshRCmd> cmd_) noexcept {}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::BindStaticMeshRCmd> cmd_) noexcept {}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::BindStorageBufferRCmd> cmd_) noexcept {

    const auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    static_cast<VkState*>(state_)->srt.bind(cmd_->_symbolId, cmd_->_storageView);
}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::BindTextureRCmd> cmd_) noexcept {

    const auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    static_cast<VkState*>(state_)->srt.bind(cmd_->_symbolId, cmd_->_sampledTexture);
}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::BindUniformBufferRCmd> cmd_) noexcept {

    const auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    static_cast<VkState*>(state_)->srt.bind(cmd_->_symbolId, cmd_->_uniformView);
}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::DrawMeshRCmd> cmd_) noexcept {

    auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    /* Ensure Resources */

    auto& srt = static_cast<VkState*>(state_)->srt;

    const auto activePipe = srt.getActivePipeline();
    assert(activePipe);

    Vector<_::VkDescriptorSet> descriptorSets {};
    const auto result = srt.commit(activePipe->getBindingLayout(), descriptorSets);
    assert(result);

    active->bindDescriptor(reinterpret_cast<ref<Vector<::vk::DescriptorSet>>>(descriptorSets));

    /**/

    constexpr static auto assumeTriangleTopology = 3uL;

    if (cmd_->_indexedPrimitive) {

        active->drawIndexed(
            cmd_->_instanceCount,
            cmd_->_instanceOffset,
            cmd_->_primitiveCount * assumeTriangleTopology,
            cmd_->_primitiveOffset * assumeTriangleTopology,
            0uL
        );

    } else {

        active->draw(
            cmd_->_instanceCount,
            cmd_->_instanceOffset,
            cmd_->_primitiveCount * assumeTriangleTopology,
            cmd_->_primitiveOffset * assumeTriangleTopology
        );
    }
}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::DrawSkeletalMeshRCmd> cmd_) noexcept {}

void VkRCmdTranslator::translate(const ptr<State> state_, const ptr<const cmd::DrawDispatchRCmd> cmd_) noexcept {

    auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    /* Ensure Resources */

    auto& srt = static_cast<VkState*>(state_)->srt;

    const auto activePipe = srt.getActivePipeline();
    assert(activePipe);

    Vector<_::VkDescriptorSet> descriptorSets {};
    const auto result = srt.commit(activePipe->getBindingLayout(), descriptorSets);
    assert(result);

    active->bindDescriptor(reinterpret_cast<ref<Vector<::vk::DescriptorSet>>>(descriptorSets));

    /**/

    if (cmd_->_indexedPrimitive) {

        active->drawIndexed(
            cmd_->_instanceCount,
            cmd_->_instanceOffset,
            cmd_->_vertexCount,
            cmd_->_vertexOffset,
            0uL
        );

    } else {

        active->draw(
            cmd_->_instanceCount,
            cmd_->_instanceOffset,
            cmd_->_vertexCount,
            cmd_->_vertexOffset
        );
    }
}

void VkRCmdTranslator::translate(
    const ptr<State> state_,
    const ptr<const cmd::DrawDispatchIndirectRCmd> cmd_
) noexcept {

    auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    if (cmd_->indexedPrimitive) {

        active->drawIndexedIndirect(
            /*cmd_->payload*/ {},
            cmd_->count,
            0uL
        );

    } else {

        active->drawIndirect(
            /*cmd_->payload*/ {},
            cmd_->count,
            0uL
        );

    }
}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::LambdaRCmd> cmd_) noexcept {

    auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    (cmd_->fn)(active.value());
}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::AttachResourceRCmd> cmd_) noexcept {

    auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    static_cast<VkState*>(state_)->srt.attach(cmd_->attached());

}
