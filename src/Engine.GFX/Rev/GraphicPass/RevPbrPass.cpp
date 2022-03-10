#include "RevPbrPass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "RevDepthPass.hpp"
#include "RevPbrPassSkeletalStage.hpp"
#include "RevPbrPassStaticStage.hpp"
#include "../../Graphics.hpp"
#include "../../Texture/TextureFactory.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RevPbrPass::RevPbrPass(cref<sptr<Device>> device_, const ptr<RevDepthPass> depthPass_) :
    GraphicPass(device_, GraphicPassMask::ePbrPass),
    _depthPass(depthPass_) {}

void RevPbrPass::setup() {

    SCOPED_STOPWATCH

    auto pbrStatic = new RevPbrPassStaticStage { this };
    pbrStatic->setup();
    _pipeline.add(pbrStatic);

    auto pbrSkeletal = new RevPbrPassSkeletalStage { this };
    pbrSkeletal->setup();
    //_pipeline.add(pbrSkeletal);
}

void RevPbrPass::destroy() {
    auto& stages = _pipeline.stages();
    for (auto it { stages.rbegin() }; it != stages.rend(); ++it) {

        auto& stage { *it };

        stage->destroy();
        delete stage;
        stage = nullptr;
    }

    stages.clear();
}

void RevPbrPass::allocateWith(const ptr<const RenderInvocation> invocation_, const ptr<RenderInvocationState> state_) {

    const auto* factory { TextureFactory::get() };
    const auto* firstPbrStage { static_cast<ptr<RevPbrPassStaticStage>>(_pipeline.stages()[0]) };

    Framebuffer buffer { _device };
    buffer.setExtent(invocation_->target()->extent());
    buffer.setRenderPass(firstPbrStage->renderPass());

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
    const auto depthEntry { state_->data.at("RevDepthPass::Framebuffer"sv) };
    auto& depthFrame { *_STD static_pointer_cast<Framebuffer, void>(depthEntry) };

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
    state_->data.insert_or_assign("RevPbrPass::Framebuffer"sv, _STD make_shared<decltype(buffer)>(_STD move(buffer)));

    /**
     *
     */
    GraphicPass::allocateWith(invocation_, state_);
}

void RevPbrPass::freeWith(const ptr<const RenderInvocation> invocation_, const ptr<RenderInvocationState> state_) {
    GraphicPass::freeWith(invocation_, state_);

    /**
     *
     */
    const auto it { state_->data.find("RevPbrPass::Framebuffer"sv) };
    if (it != state_->data.end()) {

        sptr<Framebuffer> buffer {
            _STD static_pointer_cast<Framebuffer, void>(it->second)
        };

        /**
         *
         */
        buffer->destroy();

        for (auto& entry : buffer->attachments()) {
            entry->destroy();
        }

        /**
         *
         */
        state_->data.erase(it);
    }
}

ptr<GraphicPassModelProcessor> RevPbrPass::processor() noexcept {
    return nullptr;
}
