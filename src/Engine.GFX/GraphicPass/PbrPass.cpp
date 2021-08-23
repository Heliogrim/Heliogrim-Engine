#include "PbrPass.hpp"

#include "DepthPass.hpp"
#include "PbrPassSkeletalStage.hpp"
#include "PbrPassStaticStage.hpp"
#include "../Graphics.hpp"
#include "../Texture/TextureFactory.hpp"

using namespace ember::engine::gfx;
using namespace ember;

PbrPass::PbrPass() :
    GraphicPass(GraphicPassMask::ePbrPass) {}

void PbrPass::setup() {

    auto pbrStatic = new PbrPassStaticStage { this };
    pbrStatic->setup();
    _pipeline.add(pbrStatic);

    auto pbrSkeletal = new PbrPassSkeletalStage { this };
    pbrSkeletal->setup();
    _pipeline.add(pbrSkeletal);

    const auto device = Graphics::get()->getCurrentDevice();
    const auto swapchain = Graphics::get()->getCurrentSwapchain();
    const auto factory = TextureFactory::get();
    auto depthPass = static_cast<ptr<DepthPass>>(Graphics::get()->graphicPass(GraphicPassMask::eDepthPass));

    _framebuffers.clear();
    _framebuffers.reserve(swapchain->length());

    for (u32 i = 0; i < swapchain->length(); ++i) {
        /**
         *
         */
        Framebuffer buffer { device };

        buffer.setExtent(math::uivec3 { swapchain->extent(), 1ui32 });
        buffer.setRenderPass(pbrStatic->renderPass());

        /**
         *
         */
        auto albedo = factory->build({
            buffer.extent(),
            TextureFormat::eR8G8B8A8Unorm,
            1ui32,
            TextureType::e2d,
            vk::ImageAspectFlagBits::eColor,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            vk::SharingMode::eExclusive
        });

        auto normal = factory->build({
            buffer.extent(),
            TextureFormat::eR32G32B32A32Sfloat,
            1ui32,
            TextureType::e2d,
            vk::ImageAspectFlagBits::eColor,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            vk::SharingMode::eExclusive
        });

        auto mrs = factory->build({
            buffer.extent(),
            TextureFormat::eR16G16B16A16Sfloat,
            1ui32,
            TextureType::e2d,
            vk::ImageAspectFlagBits::eColor,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            vk::SharingMode::eExclusive
        });

        factory->buildView(albedo);
        factory->buildView(normal);
        factory->buildView(mrs);

        /**
         *
         */
        auto& depthFrame = depthPass->framebuffer(i);
        cref<FramebufferAttachment> depth = depthFrame.attachments()[0];

        /**
         *
         */
        buffer.add(_STD move(FramebufferAttachment { _STD move(albedo) }));
        buffer.add(_STD move(FramebufferAttachment { _STD move(normal) }));
        buffer.add(_STD move(FramebufferAttachment { _STD move(mrs) }));
        buffer.add(depth);

        /**
         *
         */
        buffer.setup();
        _framebuffers.push_back(_STD move(buffer));
    }
}

void PbrPass::destroy() {

    for (auto& entry : _framebuffers) {

        for (auto& attachment : entry.attachments()) {
            attachment->destroy();
        }

        entry.destroy();
    }

    _framebuffers.clear();

    auto& stages = _pipeline.stages();
    for (auto& entry : stages) {
        delete entry;
        entry = nullptr;
    }
}

ptr<ModelPassProcessor> PbrPass::processor() noexcept {
    return nullptr;
}

cref<Framebuffer> PbrPass::framebuffer(u32 idx_) const {
    return _framebuffers[idx_];
}

cref<Framebuffer> PbrPass::currentFramebuffer() const noexcept {
    return _framebuffers[Graphics::get()->getCurrentSwapchain()->currentIdx()];
}
