#include "DepthPass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "DepthPassSkeletalStage.hpp"
#include "DepthPassStaticStage.hpp"
#include "../Graphics.hpp"
#include "../Texture/TextureFactory.hpp"

using namespace ember::engine::gfx;
using namespace ember;

DepthPass::DepthPass() :
    GraphicPass(GraphicPassMask::eDepthPass),
    _processor({ this }) {}

void DepthPass::setup() {

    SCOPED_STOPWATCH

    auto depthStage = new DepthPassStaticStage { this };
    depthStage->setup();
    _pipeline.add(depthStage);

    auto depthStageSkeletal = new DepthPassSkeletalStage { this };
    depthStageSkeletal->setup();
    _pipeline.add(depthStageSkeletal);

    const auto device = Graphics::get()->getCurrentDevice();
    const auto swapchain = Graphics::get()->getCurrentSwapchain();
    const auto factory = TextureFactory::get();

    _framebuffers.clear();
    _framebuffers.reserve(swapchain->length());

    for (u32 i = 0; i < swapchain->length(); ++i) {
        /**
         *
         */
        Framebuffer buffer { device };

        buffer.setExtent(math::uivec3 { swapchain->extent(), 1ui32 });
        buffer.setRenderPass(depthStage->renderPass());

        /**
         *
         */
        auto texture = factory->build({
            buffer.extent(),
            TextureFormat::eD32SfloatS8Uint,
            1ui32,
            TextureType::e2d,
            vk::ImageAspectFlagBits::eDepth,
            vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            vk::SharingMode::eExclusive
        });

        factory->buildView(texture);

        /**
         *
         */
        buffer.add(_STD move(
            FramebufferAttachment {
                _STD move(texture)
            }
        ));

        /**
         *
         */
        buffer.setup();
        _framebuffers.push_back(_STD move(buffer));
    }
}

void DepthPass::destroy() {

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

ptr<ModelPassProcessor> DepthPass::processor() noexcept {
    _processor.reset();
    return &_processor;
}

cref<Framebuffer> DepthPass::framebuffer(u32 idx_) const {
    return _framebuffers[idx_];
}

cref<Framebuffer> DepthPass::currentFramebuffer() const noexcept {
    return _framebuffers[Graphics::get()->getCurrentSwapchain()->currentIdx()];
}
