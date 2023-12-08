#include "DummyProvider.hpp"

#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneDepth.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::render;
using namespace hg::engine::gfx;
using namespace hg;

void DummyProvider::destroy() noexcept {
    SubPass::destroy();

    _resources.outDepthTexture->destroy<smr<TextureLikeObject>, graph::TextureDescription>();

    _depthTexture->destroy();
    _depthTexture.reset();
}

void DummyProvider::declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    symCtx_.registerExposeSymbol(makeSceneDepthSymbol(), &_resources.outDepthTexture);
}

void DummyProvider::iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept {}

void DummyProvider::resolve() noexcept {}

void DummyProvider::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    auto sceneColor = symCtx_.getExportSymbol(makeSceneColorSymbol());
    const auto& sceneColorTex = sceneColor->load<smr<const TextureLikeObject>, graph::TextureDescription>();

    assert(not sceneColor->empty() && not sceneColorTex.empty());

    auto sct = clone(sceneColorTex).into<Texture>();
    if (not _depthTexture.empty() && _depthTexture->extent() != sct->extent()) {
        _depthTexture->destroy();
        _depthTexture.reset();
    }

    if (_depthTexture.empty()) {

        const auto* const factory = TextureFactory::get();
        auto sceneDepthTex = factory->build(
            {
                sct->extent(), TextureFormat::eD32Sfloat, 1uL, TextureType::e2d, vk::ImageAspectFlagBits::eDepth,
                vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
                vk::SharingMode::eExclusive
            }
        );
        factory->buildView(sceneDepthTex, { .layers = { 0L, 1L }, .type = TextureType::e2d, .mipLevels = { 0L, 1L } });

        _depthTexture = make_smr<Texture>(_STD move(sceneDepthTex));

        if (_resources.outDepthTexture->empty()) {

            _resources.outDepthTexture.obj()->symbol = makeSceneDepthSymbol();
            _resources.outDepthTexture.obj()->owned = true;

            _resources.outDepthTexture->create<
                smr<TextureLikeObject>,
                graph::TextureDescription,
                decltype([](const auto& obj_) {
                    return obj_.get();
                })
            >(clone(_depthTexture).into<TextureLikeObject>());
        } else {
            _resources.outDepthTexture->store<
                smr<TextureLikeObject>,
                graph::TextureDescription,
                decltype([](const auto& obj_) {
                    return obj_.get();
                })
            >(clone(_depthTexture).into<TextureLikeObject>());
        }

        /**/
        const_cast<ref<graph::ScopedSymbolContext>>(symCtx_).eraseExposedSymbol(makeSceneDepthSymbol());
        const_cast<ref<graph::ScopedSymbolContext>>(symCtx_).exposeSymbol(
            makeSceneDepthSymbol(),
            _resources.outDepthTexture.obj()
        );
    }
}
