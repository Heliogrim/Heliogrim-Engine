#include "VkExecutionTranslationUnit.hpp"

#include <ranges>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.GFX.Render.Command/Commands/BindIndexBuffer.hpp>
#include <Engine.GFX.Render.Command/Commands/BindSkeletalMesh.hpp>
#include <Engine.GFX.Render.Command/Commands/BindStaticMesh.hpp>
#include <Engine.GFX.Render.Command/Commands/BindVertexBuffer.hpp>
#include <Engine.GFX.Render.Command/Commands/DrawMesh.hpp>
#include <Engine.GFX/Geometry/PolygonMesh.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/Pass/AccelerationPassFactory.hpp>
#include <Engine.GFX.Material/Material.hpp>
#include <Engine.GFX.Material/MaterialParameterStorageTable.hpp>
#include <Engine.GFX.Material/MaterialPrototypeParameter.hpp>
#include <Engine.GFX.Acc/Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.Reflect/IsType.hpp>
#include <Engine.GFX.Acc/Pipeline/Layout/AccelerationBindingLayout.hpp>
#include <Engine.GFX.Acc/Pipeline/Layout/AccelerationPassBindings.hpp>
#include <Engine.GFX.Acc/Symbol/Symbol.hpp>
#include <Engine.GFX.Acc.Compile/EffectCompileResult.hpp>

#include "VkDescriptorWriter.hpp"

#ifdef _DEBUG
#include <Engine.Logging/Logger.hpp>
#endif

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx::render::cmd;
using namespace hg;

VkExecutionTranslationUnit::VkExecutionTranslationUnit(mref<uptr<Resolver>> resolver_) noexcept :
    ExecutionTranslationUnit(_STD move(resolver_)) {}

VkExecutionTranslationUnit::~VkExecutionTranslationUnit() noexcept = default;

void VkExecutionTranslationUnit::translate(ptr<const cmd::BeginRenderCommand>) noexcept {
    #ifdef _DEBUG
    if (not _inversions.empty()) {
        IM_CORE_ERROR("Failed to validate empty control inversion set while translating render commands.");
    }
    _inversions.clear();
    #endif

    #ifdef _DEBUG
    if (_bdctx.indexBuffer.size() != 0) {
        IM_CORE_ERROR("Failed to validate empty dispatch context while translating render commands.");
    }
    _bdctx.indexBuffer.clear();
    #endif

    #ifdef _DEBUG
    if (_bdctx.vertexBuffer.size() != 0) {
        IM_CORE_ERROR("Failed to validate empty dispatch context while translating render commands.");
    }
    _bdctx.vertexBuffer.clear();
    #endif
}

void VkExecutionTranslationUnit::translate(ptr<const cmd::NextSubpassRenderCommand>) noexcept {
    _inversions.clear();
    _bdctx.indexBuffer.clear();
    _bdctx.vertexBuffer.clear();
}

void VkExecutionTranslationUnit::translate(ptr<const cmd::EndRenderCommand>) noexcept {
    _inversions.clear();
    _bdctx.indexBuffer.clear();
    _bdctx.vertexBuffer.clear();
}

void VkExecutionTranslationUnit::translate(
    ptr<const cmd::BindIndexBufferRenderCommand> cmd_
) noexcept {
    const auto* const indexView = cmd_->_indexView;
    _bdctx.indexBuffer.emplace_back(indexView);
}

void VkExecutionTranslationUnit::translate(
    ptr<const cmd::BindVertexBufferRenderCommand> cmd_
) noexcept {
    const auto* const vertexView = cmd_->_vertexView;
    _bdctx.vertexBuffer.emplace_back(vertexView);
}

void VkExecutionTranslationUnit::translate(
    ptr<const cmd::BindMaterialRenderCommand> bindMaterialRenderCommand_
) noexcept {
    ExecutionTranslationUnit::translate(bindMaterialRenderCommand_);
}

void VkExecutionTranslationUnit::translate(
    ptr<const cmd::BindSkeletalMeshRenderCommand> cmd_
) noexcept {

    const auto* const mesh = cmd_->_skeletalMesh;
    const auto* const poly = static_cast<const ptr<const PolygonMesh>>(mesh);

    IndexBufferView* indexView {};
    // TODO: indexView = *poly->indices();
    _bdctx.indexBuffer.emplace_back(indexView);

    VertexBufferView* vertexView {};
    // TODO: vertexView = *poly->vertices();
    _bdctx.vertexBuffer.emplace_back(vertexView);
}

void VkExecutionTranslationUnit::translate(
    ptr<const cmd::BindStaticMeshRenderCommand> cmd_
) noexcept {

    const auto* const mesh = cmd_->_staticMesh;
    const auto* const poly = static_cast<const ptr<const PolygonMesh>>(mesh);

    IndexBufferView* indexView {};
    // TODO: indexView = *poly->indices();
    _bdctx.indexBuffer.emplace_back(indexView);

    VertexBufferView* vertexView {};
    // TODO: vertexView = *poly->vertices();
    _bdctx.vertexBuffer.emplace_back(vertexView);
}

void VkExecutionTranslationUnit::translate(
    ptr<const cmd::BindStorageBufferRenderCommand> bindStorageBufferRenderCommand_
) noexcept {
    ExecutionTranslationUnit::translate(bindStorageBufferRenderCommand_);
}

void VkExecutionTranslationUnit::translate(
    ptr<const cmd::BindTextureRenderCommand> bindTextureRenderCommand_
) noexcept {
    ExecutionTranslationUnit::translate(bindTextureRenderCommand_);
}

void VkExecutionTranslationUnit::translate(ptr<const cmd::DrawMeshRenderCommand> cmd_) noexcept {

    assert(_bdctx.vertexBuffer.size());
    assert(not _inversions.empty());

    if (cmd_->_indexedPrimitive) {
        assert(_bdctx.indexBuffer.size());
    }

    for (const auto& inversion : _inversions) {

        assert(inversion.type == InversionControlType::eMaterial);

        const auto* const material = static_cast<const ptr<const material::Material>>(inversion.data);
        const auto materialProto = material->getPrototype();
        const auto resolved = _resolver->resolveCompiledMaterial(material);

        assert(resolved->flag == acc::EffectCompileResultFlag::eCompiled);
        assert(IsType<acc::VkGraphicsPipeline>(*resolved->apass));
        const auto* const graphicsPipeline = static_cast<const ptr<const acc::VkGraphicsPipeline>>(resolved->apass.
            get());

        AccelerationCommandBuffer* dummy {};
        AccelerationCommandBuffer acmd = *dummy;

        // Error: We have to build the actual pass before we start translating the commands
        // Error:   We need the contextual information of the graph node to work correctly.
        acc::AccelerationPassFactory* passFactory {};
        Vector<smr<const acc::Symbol>> outputSymbols {};
        Vector<smr<const acc::Symbol>> inputSymbols {};
        const auto graphicsPass = passFactory->buildGraphicsPass(outputSymbols, inputSymbols);

        acmd.begin();
        // TODO: acmd.beginRenderPass();

        // TODO: acmd.bindPipeline();

        if (cmd_->_indexedPrimitive) {
            const ptr<const VirtualBufferView> buffer {};
            acmd.bindIndexBuffer(buffer);
        }

        {
            u64 idx = 0;
            for (const auto& vertexBuffer : _bdctx.vertexBuffer) {
                const ptr<const VirtualBufferView> buffer {};
                // TODO: buffer = vertexBuffer;
                acmd.bindVertexBuffer(static_cast<u32>(idx++), buffer);
            }
        }

        /* Resolve provided parameters of material and bind to inversion pipeline */

        {
            const auto& layouts = graphicsPipeline->getPassBindings().layouts;

            u32 layoutIdx = 0uL;
            for (const acc::AccelerationBindingLayout& layout : layouts) {

                vk::DescriptorSet set {};
                VkDescriptorWriter writer { set };

                u32 setIdx = 0uL;
                for (const acc::AccelerationBindingLayoutElement& element : layout.elements) {

                    auto remapped = resolved->remapping.alias(element.token);
                    assert(remapped.has_value());

                    // TODO: const auto symbol = remapped.has_value() ? _STD move(remapped.value()) : defaultImportSymbol;
                    const auto symbol = _STD move(remapped.value());

                    switch (element.type) {
                        case acc::AccelerationBindingType::eStorageBuffer: {

                            const auto proto = materialProto->getParameter(symbol->name);
                            assert(proto.has_value());
                            assert(proto.value().getDataType() == acc::TransferDataType::eStorage);

                            const auto val = material->getParam<smr<void>>(symbol->name);

                            // Error:
                            Buffer buffer {};
                            writer.storeStorage(setIdx, buffer);

                            break;
                        }
                        case acc::AccelerationBindingType::eTexture: {

                            const auto proto = materialProto->getParameter(symbol->name);
                            assert(proto.has_value());
                            assert(proto.value().getDataType() == acc::TransferDataType::eSampler);

                            const auto val = material->getParam<smr<TextureResource>>(symbol->name);
                            const auto guard = val->acquire(resource::ResourceUsageFlag::eRead);

                            const auto proxy = guard.imm();

                            if (auto tex = proxy->as<Texture>()) {
                                // Error:
                                writer.store(setIdx, *tex, nullptr);
                            } else if (auto virTex = proxy->as<VirtualTexture>()) {
                                // Error:
                                writer.store(setIdx, virTex, nullptr);
                            } else if (auto virTexView = proxy->as<VirtualTextureView>()) {
                                // Error:
                                writer.store(setIdx, virTexView, nullptr);
                            }

                            break;
                        }
                        case acc::AccelerationBindingType::eUniformBuffer: {

                            const auto proto = materialProto->getParameter(symbol->name);
                            assert(proto.has_value());
                            assert(proto.value().getDataType() == acc::TransferDataType::eUniform);

                            const auto val = material->getParam<smr<TextureResource>>(symbol->name);

                            // Error:
                            Buffer buffer {};
                            writer.storeUniform(setIdx, buffer);

                            break;
                        }
                    }

                    ++setIdx;
                }

                // Error:
                vk::Device device {};
                writer.update(device);
                acmd.bindDescriptor(layoutIdx++, set);
            }
        }

        acmd.endRenderPass();
        acmd.end();

        _building.push_back(acmd);
    }
}

void VkExecutionTranslationUnit::translate(
    ptr<const cmd::DrawSkeletalMeshRenderCommand> drawSkeletalMeshRenderCommand_
) noexcept {
    ExecutionTranslationUnit::translate(drawSkeletalMeshRenderCommand_);
}
