#include "RevDepthPass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "RevDepthPassSkeletalStage.hpp"
#include "RevDepthPassStaticStage.hpp"
#include "../../Graphics.hpp"
#include "../../Texture/TextureFactory.hpp"
#include "__macro.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RevDepthPass::RevDepthPass(cref<sptr<Device>> device_) :
    GraphicPass(device_, GraphicPassMask::eDepthPass),
    _processor(this) {}

void RevDepthPass::setup() {

    SCOPED_STOPWATCH

    auto depthStaticStage = new RevDepthPassStaticStage { this };
    depthStaticStage->setup();
    _pipeline.add(depthStaticStage);

    //auto depthStageSkeletal = new RevDepthPassSkeletalStage { this };
    //depthStageSkeletal->setup();
    //_pipeline.add(depthStageSkeletal);
}

void RevDepthPass::destroy() {
    auto& stages = _pipeline.stages();
    for (auto it { stages.rbegin() }; it != stages.rend(); ++it) {

        auto& stage { *it };

        stage->destroy();
        delete stage;
        stage = nullptr;
    }

    stages.clear();
}

void RevDepthPass::postProcessAllocated(const ptr<RenderPassState> state_) const {

    const auto it { state_->data.find("RevDepthPass::Framebuffer"sv) };
    if (it == state_->data.end()) {
        return;
    }

    const ptr<Framebuffer> buffer {
        static_cast<const ptr<Framebuffer>>(it->second.get())
    };
    auto& depthImage { *buffer->attachments().at(0).unwrapped() };

    const vk::ImageMemoryBarrier imgBarrier {
        vk::AccessFlags {},
        vk::AccessFlagBits::eShaderRead,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        depthImage.buffer().image(),
        vk::ImageSubresourceRange {
            (isDepthFormat(depthImage.format()) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eNoneKHR) |
            (isStencilFormat(depthImage.format()) ? vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eNoneKHR),
            0,
            depthImage.mipLevels(),
            0,
            depthImage.layer()
        }

    };

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
        1, &imgBarrier
    );

    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();
}

void RevDepthPass::allocateWith(const ptr<const RenderPass> invocation_,
    const ptr<RenderPassState> state_) {

    const auto* factory { TextureFactory::get() };
    const auto* firstDepthStage { static_cast<ptr<RevDepthPassStaticStage>>(_pipeline.stages()[0]) };

    Framebuffer buffer { _device };
    buffer.setExtent(invocation_->target()->extent());
    buffer.setRenderPass(firstDepthStage->renderPass());

    /**
     *
     */
    auto texture {
        factory->build({
            buffer.extent(),
            REV_DEPTH_FORMAT,
            1ui32,
            TextureType::e2d,
            vk::ImageAspectFlagBits::eDepth/* | vk::ImageAspectFlagBits::eStencil*/,
            vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            vk::SharingMode::eExclusive
        })
    };

    factory->buildView(texture);

    /**
     *
     */
    buffer.add({ _STD move(texture) });

    /**
     *
     */
    buffer.setup();
    state_->data.insert_or_assign("RevDepthPass::Framebuffer"sv, _STD make_shared<decltype(buffer)>(_STD move(buffer)));

    /**
     *
     */
    GraphicPass::allocateWith(invocation_, state_);

    /**
     * Post Process Images
     */
    postProcessAllocated(state_);
}

void RevDepthPass::freeWith(const ptr<const RenderPass> invocation_, const ptr<RenderPassState> state_) {
    GraphicPass::freeWith(invocation_, state_);

    /**
     *
     */
    const auto it { state_->data.find("RevDepthPass::Framebuffer"sv) };
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

ptr<GraphicPassModelProcessor> RevDepthPass::processor() noexcept {
    _processor.reset();
    return &_processor;
}
