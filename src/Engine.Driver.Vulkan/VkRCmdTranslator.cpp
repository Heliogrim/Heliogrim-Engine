#include "VkRCmdTranslator.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.GFX.Acc/Pass/VkGraphicsPass.hpp>
#include <Engine.GFX.Acc/Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.GFX.Render.Command/RenderCommandIterator.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Render.Command/Commands/BeginAccelerationPass.hpp>
#include <Engine.GFX.Render.Command/Commands/BindIndexBuffer.hpp>
#include <Engine.GFX.Render.Command/Commands/BindPipeline.hpp>
#include <Engine.GFX.Render.Command/Commands/BindVertexBuffer.hpp>
#include <Engine.GFX.Render.Command/Commands/DrawDispatch.hpp>
#include <Engine.GFX.Render.Command/Commands/DrawDispatchIndirect.hpp>
#include <Engine.Reflect/Cast.hpp>

using namespace hg::driver::vk;
using namespace hg::engine::gfx::render;
using namespace hg;

ptr<VkNativeBatch> VkRCmdTranslator::operator()(const ptr<const cmd::RenderCommandBuffer> commands_) noexcept {

    assert(commands_->root());

    /**/
    auto batch = new VkNativeBatch();

    auto device = engine::Engine::getEngine()->getGraphics()->getCurrentDevice();
    auto ecpool = device->graphicsQueue()->pool();
    engine::gfx::AccelCommandBuffer eccmd = ecpool->make();
    /**/

    auto* state = new VkState(driver::vk::VkScopedCmdMgr { 0u, eccmd });
    auto iter = cmd::RenderCommandIterator { commands_->root().get() };

    while (iter.valid()) {
        (*iter++)(state, this);
    }

    delete state;

    /**/
    batch->add(make_uptr<engine::gfx::AccelCommandBuffer>(_STD move(eccmd)));
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

    const auto graphicsPass = Cast<engine::gfx::acc::VkGraphicsPass>(cmd_->data.pass.get());
    assert(graphicsPass);

    active->bindRenderPass(
        {
            .graphicsPass = *graphicsPass,
            .framebuffer = cmd_->data.framebuffer.load(),
            .subrecording = false
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

    auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

    // TODO: assert(active->getFeatureSet() & cmd_->featureSet);

    const auto graphicsPipeline = Cast<engine::gfx::acc::VkGraphicsPipeline>(cmd_->pipeline.get());
    assert(graphicsPipeline);

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

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::BindStorageBufferRCmd> cmd_) noexcept {}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::BindTextureRCmd> cmd_) noexcept {}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::DrawMeshRCmd> cmd_) noexcept {}

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::DrawSkeletalMeshRCmd> cmd_) noexcept {}

void VkRCmdTranslator::translate(const ptr<State> state_, const ptr<const cmd::DrawDispatchRCmd> cmd_) noexcept {

    auto active = static_cast<VkState*>(state_)->cmd.active();
    assert(active.has_value());

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

void VkRCmdTranslator::translate(ptr<State> state_, ptr<const cmd::LambdaRCmd> cmd_) noexcept {}
