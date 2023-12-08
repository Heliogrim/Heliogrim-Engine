#include "Visualize.hpp"

#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneDepth.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>

using namespace hg::engine::render;
using namespace hg;

void Visualize::destroy() noexcept {
    SubPass::destroy();
}

void Visualize::declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    SubPass::declareOutputs(symCtx_);

    symCtx_.registerExportSymbol(makeSceneColorSymbol(), &_resources.outSceneColor);
}

void Visualize::iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept {}

void Visualize::resolve() noexcept {}

void Visualize::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    if (_selected.empty()) {
        return;
    }

    if (_available.empty() || not _available.contains(makeSceneDepthSymbol())) {
        return;

        // Warning: Will not work for depth buffers; We may have to use compute shaders to make specific resources visualized through the viewport
        /*
        _available.insert_or_assign(
            makeSceneDepthSymbol(),
            [&symCtx_]() {

                auto sceneColor = symCtx_.getExportSymbol(makeSceneColorSymbol())->load<smr<Texture>>();
                auto sceneDepth = symCtx_.getImportSymbol(makeSceneDepthSymbol())->load<smr<const Texture>>();

                cmd::RenderCommandBuffer rcb {};
                rcb.begin();
                rcb.lambda(
                    [sceneDepth, sceneColor](ref<AccelCommandBuffer> cmd_) {
                        cmd_.copyImage(
                            sceneDepth->buffer(),
                            sceneColor->buffer(),
                            vk::ImageCopy {
                                .srcSubresource = vk::ImageSubresourceLayers {
                                    .aspectMask = vk::ImageAspectFlagBits::eDepth,
                                    .mipLevel = 0uL,
                                    .baseArrayLayer = 0uL,
                                    .layerCount = 1uL
                                },
                                .srcOffset = vk::Offset3D { 0L, 0L, 0L },
                                .dstSubresource = vk::ImageSubresourceLayers {
                                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                                    .mipLevel = 0uL,
                                    .baseArrayLayer = 0uL,
                                    .layerCount = 1uL
                                },
                                .dstOffset = vk::Offset3D { 0L, 0L, 0L },
                                .extent = vk::Extent3D { sceneColor->width(), sceneColor->height(), 1uL }
                            }
                        );
                    }
                );
                rcb.end();

            }
        );
         */

    }

}
