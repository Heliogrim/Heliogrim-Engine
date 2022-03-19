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

RevPbrPass::RevPbrPass(cref<sptr<Device>> device_) :
    GraphicPass(device_, GraphicPassMask::ePbrPass),
    _processor(this) {}

void RevPbrPass::setup() {

    SCOPED_STOPWATCH

    auto pbrStaticStage = new RevPbrPassStaticStage { this };
    pbrStaticStage->setup();
    _pipeline.add(pbrStaticStage);

    //auto pbrSkeletal = new RevPbrPassSkeletalStage { this };
    //pbrSkeletal->setup();
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

void RevPbrPass::postProcessAllocated(const ptr<RenderInvocationState> state_) const {

    const auto it { state_->data.find("RevPbrPass::Framebuffer"sv) };
    if (it == state_->data.end()) {
        return;
    }

    const ptr<Framebuffer> buffer {
        static_cast<const ptr<Framebuffer>>(it->second.get())
    };

    Vector<vk::ImageMemoryBarrier> imgBarriers {};
    for (const auto& entry : buffer->attachments()) {

        auto& attachment { *entry.unwrapped() };

        if (isDepthFormat(attachment.format()) || isStencilFormat(attachment.format())) {
            continue;
        }

        imgBarriers.push_back({
            vk::AccessFlags {},
            vk::AccessFlagBits::eShaderRead,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            attachment.buffer().image(),
            vk::ImageSubresourceRange {
                vk::ImageAspectFlagBits::eColor,
                0,
                attachment.mipLevels(),
                0,
                attachment.layer()
            }

        });
    }

    auto pool = _device->graphicsQueue()->pool();
    pool->lck().acquire();
    CommandBuffer cmd = pool->make();
    cmd.begin();

    /**
     * Transform
     */
    cmd.vkCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlagBits::eAllCommands, vk::DependencyFlags {},
        0, nullptr,
        0, nullptr,
        static_cast<u32>(imgBarriers.size()), imgBarriers.data()
    );

    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();
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
    auto position = factory->build({
        buffer.extent(),
        TextureFormat::eR32G32B32A32Sfloat,
        // Surface Positions
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
        // Surface Normals
        1ui32,
        TextureType::e2d,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SharingMode::eExclusive
    });

    auto meta = factory->build({
        buffer.extent(),
        TextureFormat::eR16G16B16A16Sfloat,
        // Meta data Sampling
        1ui32,
        TextureType::e2d,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SharingMode::eExclusive
    });

    factory->buildView(position);
    factory->buildView(normal);
    factory->buildView(meta);

    /**
     *
     */
    const auto depthEntry { state_->data.at("RevDepthPass::Framebuffer"sv) };
    auto& depthFrame { *_STD static_pointer_cast<Framebuffer, void>(depthEntry) };

    cref<FramebufferAttachment> depth = depthFrame.attachments()[0];

    /**
     *
     */
    buffer.add(FramebufferAttachment { _STD move(position) });
    buffer.add(FramebufferAttachment { _STD move(normal) });
    buffer.add(FramebufferAttachment { _STD move(meta) });
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

    /**
     * Post Process Images
     */
    postProcessAllocated(state_);
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
    _processor.reset();
    return &_processor;
}
