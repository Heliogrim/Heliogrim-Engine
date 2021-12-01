#include "PbrPass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "DepthPass.hpp"
#include "PbrPassSkeletalStage.hpp"
#include "PbrPassStaticStage.hpp"
#include "../Graphics.hpp"
#include "../Texture/TextureFactory.hpp"

using namespace ember::engine::gfx;
using namespace ember;

PbrPass::PbrPass(cref<sptr<Device>> device_, const ptr<Swapchain> swapchain_, const ptr<DepthPass> depthPass_) :
    GraphicPass(device_, swapchain_, GraphicPassMask::ePbrPass),
    _depthPass(depthPass_) {}

void PbrPass::setup() {

    SCOPED_STOPWATCH

    auto pbrStatic = new PbrPassStaticStage { this };
    pbrStatic->setup();
    _pipeline.add(pbrStatic);

    auto pbrSkeletal = new PbrPassSkeletalStage { this };
    pbrSkeletal->setup();
    _pipeline.add(pbrSkeletal);

    const auto factory = TextureFactory::get();

    _framebuffers.clear();
    _framebuffers.reserve(_swapchain->length());

    for (u32 i = 0; i < _swapchain->length(); ++i) {
        /**
         *
         */
        Framebuffer buffer { _device };

        buffer.setExtent(math::uivec3 { _swapchain->extent(), 1ui32 });
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
        auto& depthFrame = _depthPass->framebuffer(i);
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
    return _framebuffers[_swapchain->currentIdx()];
}
