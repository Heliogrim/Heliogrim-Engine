#include "Graphics.hpp"

#include <Engine.Resource/FileResource.hpp>
#include <Engine.Scene/Scene.hpp>
#include <Engine.Scheduler/Async.hpp>

#include "todo.h"
#include "Command/CommandBatch.hpp"
#include "GraphicPass/DepthPass.hpp"
#include "GraphicPass/FinalPass.hpp"
#include "GraphicPass/LightPass.hpp"
#include "GraphicPass/ProbePass.hpp"
#include "Loader/TextureLoader.hpp"
#include "Shader/ShaderStorage.hpp"
#include "Swapchain/Swapchain.hpp"
#include "Swapchain/VkSwapchain.hpp"
#include "Texture/VkTextureFactory.hpp"

using namespace ember::engine::gfx;
using namespace ember::engine;
using namespace ember;

ptr<Graphics> Graphics::_instance = nullptr;

/*
void tmp_load_material(string url_, cref<sptr<Device>> device_, Texture& dst_, u32 layer_) {

    auto src = loader::TextureLoader::get().load(url_);
    src.finally([&, layer_ = layer_](Texture&& texture_) -> void {

        auto pool = device_->transferQueue()->pool();

        pool->lck().acquire();
        CommandBuffer cmd = pool->make();

        cmd.begin();

        vk::ImageMemoryBarrier simb {
            vk::AccessFlags(),
            vk::AccessFlags(),
            vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::ImageLayout::eTransferDstOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            dst_.buffer().image(),
            {
                vk::ImageAspectFlagBits::eColor,
                0,
                dst_.mipLevels(),
                0,
                dst_.layer()
            }
        };

        dst_.buffer()._vkLayout = vk::ImageLayout::eTransferDstOptimal;

        cmd.vkCommandBuffer().pipelineBarrier(
            vk::PipelineStageFlagBits::eAllCommands,
            vk::PipelineStageFlagBits::eAllCommands,
            vk::DependencyFlags(),
            0, nullptr,
            0, nullptr,
            1, &simb
        );

        for (u32 level = 0; level < dst_.mipLevels(); ++level) {
            cmd.copyImage(texture_.buffer(), dst_.buffer(), {
                {
                    vk::ImageAspectFlagBits::eColor,
                    level,
                    0,
                    1
                },
                {},
                {
                    vk::ImageAspectFlagBits::eColor,
                    level,
                    layer_,
                    1
                },
                {},
                {
                    texture_.width() >> level,
                    texture_.height() >> level,
                    texture_.depth()
                }
            });
        }

        vk::ImageMemoryBarrier eimb {
            vk::AccessFlags(),
            vk::AccessFlags(),
            vk::ImageLayout::eTransferDstOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            dst_.buffer().image(),
            {
                vk::ImageAspectFlagBits::eColor,
                0,
                dst_.mipLevels(),
                0,
                dst_.layer()
            }
        };

        cmd.vkCommandBuffer().pipelineBarrier(
            vk::PipelineStageFlagBits::eAllCommands,
            vk::PipelineStageFlagBits::eAllCommands,
            vk::DependencyFlags(),
            0, nullptr,
            0, nullptr,
            1, &eimb
        );

        cmd.end();
        cmd.submitWait();
        cmd.release();

        pool->lck().release();

        dst_.buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        texture_.destroy();
    });

    scheduler::exec(scheduler::task::make_task(src));
}
 */

ptr<Graphics> Graphics::get() noexcept {
    return Graphics::_instance;
}

ptr<Graphics> Graphics::make() {
    if (!Graphics::_instance) {
        _instance = new Graphics();
    }
    return _instance;
}

Graphics::~Graphics() = default;

void Graphics::setup() {
    /**
     * Create a new application
     */
    _application = {};
    _application.setup();

    /**
     * Create a new Surface (aka. Window)
     */
    _surface = { &_application };
    _surface.setup();

    /**
     * Prepare rendering specific data structures
     *
     *	a: create a new device to access graphics architecture  
     */
    _device = make_sptr<Device>(_application, &_surface);
    _device->setup();

    _computeQueue = _device->computeQueue();
    _graphicsQueue = _device->graphicsQueue();
    _transferQueue = _device->transferQueue();

    /**
     * Create Utilities
     */
    VkTextureFactory::make(_device);
    loader::TextureLoader::make(_device);

    /**
     * Create a new swapchain to present image
     */
    _swapchain = new VkSwapchain(_device, _surface);
    _swapchain->setup();

    /**
     * Create Shader Storage
     */
    ShaderStorage::make();
    build_shader(_device);

    /**
     * Create Graphic Passes
     */
    _graphicPasses.resize(static_cast<u32>(GraphicPassMask::eFinalPass) + 1, nullptr);

    _graphicPasses[static_cast<u8>(GraphicPassMask::eDepthPass)] = new DepthPass();
    _graphicPasses[static_cast<u8>(GraphicPassMask::eLightPass)] = new LightPass();
    _graphicPasses[static_cast<u8>(GraphicPassMask::eProbePass)] = new ProbePass();
    _graphicPasses[static_cast<u8>(GraphicPassMask::ePbrPass)] = new ProbePass();
    _graphicPasses[static_cast<u8>(GraphicPassMask::eFinalPass)] = new FinalPass();

    /**
     *
     */
    for (auto& entry : _graphicPasses) {
        entry->setup();
    }

    /**
     * Prepare Synchronization of Passes
     */
    {
        constexpr GraphicPassMask masks[] {
            GraphicPassMask::eDepthPass,
            GraphicPassMask::eLightPass,
            GraphicPassMask::eProbePass,
            GraphicPassMask::ePbrPass
        };
        constexpr u8 length = static_cast<u8>(sizeof(masks)) - 1;

        _graphicPassFences.resize(length);

        for (u8 idx = 0; idx < length; ++idx) {
            _graphicPassFences[idx] = _device->vkDevice().createFence({});
        }
    }

    /**
     *
     */
    _graphicPassBatches.resize(static_cast<u32>(GraphicPassMask::eFinalPass) + 1ui32, {});

    /**
     * Prepare On Flight Synchronization
     */
    {
        _onFlightSync.image.resize(_swapchain->length());
        _onFlightSync.finish.resize(_swapchain->length());
        _onFlightSync.cpuGpuSync.resize(_swapchain->length());

        const vk::SemaphoreCreateInfo si {};
        const vk::FenceCreateInfo fi {};

        for (u32 idx = 0; idx < _swapchain->length(); ++idx) {
            _onFlightSync.image[idx] = _device->vkDevice().createSemaphore(si);
            _onFlightSync.finish[idx] = _device->vkDevice().createSemaphore(si);
            _onFlightSync.cpuGpuSync[idx] = _device->vkDevice().createFence(fi);

            #ifdef _DEBUG
            assert(_onFlightSync.image[idx]);
            assert(_onFlightSync.finish[idx]);
            assert(_onFlightSync.cpuGpuSync[idx]);
            #endif

            /**
             * Submit Fence to prevent error on first frame
             */
            _graphicsQueue->submit(CommandBatch {}, _onFlightSync.cpuGpuSync[idx]);
        }
    }
}

void Graphics::schedule() {
    scheduler::exec(
        make_repetitive_task([this]() {
            _tick();
            return true;
        }, scheduler::task::TaskMask::eCritical)
    );
}

void Graphics::destroy() {

    /**
     *
     */
    for (auto& entry : _graphicPassFences) {
        _device->vkDevice().destroyFence(entry);
    }

    for (auto& entry : _graphicPasses) {
        entry->destroy();
    }

    _graphicPasses.clear();

    /**
     *
     */
    for (u32 idx = 0; idx < _swapchain->length(); ++idx) {
        _device->vkDevice().destroy(_onFlightSync.image[idx]);
        _onFlightSync.image[idx] = nullptr;

        _device->vkDevice().destroy(_onFlightSync.finish[idx]);
        _onFlightSync.finish[idx] = nullptr;

        _device->vkDevice().destroy(_onFlightSync.cpuGpuSync[idx]);
        _onFlightSync.cpuGpuSync[idx] = nullptr;
    }
    _onFlightSync.image.clear();
    _onFlightSync.finish.clear();
    _onFlightSync.cpuGpuSync.clear();

    /**
     * Swapchain
     */
    delete _swapchain;
    _swapchain = nullptr;

    /**
     * Device
     */
    _device->destroy();
    _device.reset();

    _surface.destroy();
    _application.destroy();
}

sptr<Device> Graphics::getCurrentDevice() const noexcept {
    return _device;
}

ptr<Swapchain> Graphics::getCurrentSwapchain() const noexcept {
    return _swapchain;
}

CommandQueue::reference_type Graphics::getComputeQueue() const noexcept {
    return *_device->computeQueue();
}

CommandQueue::reference_type Graphics::getGraphicsQueue() const noexcept {
    return *_device->graphicsQueue();
}

CommandQueue::reference_type Graphics::getTransferQueue() const noexcept {
    return *_device->transferQueue();
}

static u64 frame = 0;

void Graphics::_tick() {
    /**
     * 
     */
    static u32 syncIdx = 0;

    // TODO: Make timeout as small as possible to skip blocking request -> if failed, return an reschedule tick task
    assert(
        _device->vkDevice().waitForFences(1, &_onFlightSync.cpuGpuSync[syncIdx], VK_TRUE, UINT64_MAX) ==
        vk::Result::eSuccess
    );
    _device->vkDevice().resetFences(1, &_onFlightSync.cpuGpuSync[syncIdx]);

    /**
     * 
     */
    u32 swapIdx;
    [[maybe_unused]] auto result = _device->vkDevice().acquireNextImageKHR(
        static_cast<ptr<VkSwapchain>>(_swapchain)->vkSwapchain(),
        UINT64_MAX,
        _onFlightSync.image[syncIdx],
        nullptr,
        &swapIdx
    );

    _swapchain->setCurrentIdx(swapIdx);

    /**
     *
     */
    processGraphicPasses();

    /**
     * 
     */
    auto& batch = _graphicPassBatches[static_cast<u8>(GraphicPassMask::eFinalPass)];
    auto finalPass = _graphicPasses[static_cast<u8>(GraphicPassMask::eFinalPass)];

    batch.reset();
    finalPass->process(*scene::Scene::get()->graphs().front(), batch);

    /**
     * Execute Command Batch
     */

    batch.pushBarrier(_onFlightSync.image[syncIdx]);
    batch.pushSignal(_onFlightSync.finish[syncIdx]);
    batch.barrierStages() = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    _graphicsQueue->submit(batch, _onFlightSync.cpuGpuSync[syncIdx]);

    /**
     * Display Image
     */
    const vk::PresentInfoKHR present {
        1,
        &_onFlightSync.finish[syncIdx],
        1,
        &static_cast<ptr<VkSwapchain>>(_swapchain)->vkSwapchain(),
        &swapIdx
    };
    assert(_graphicsQueue->vkQueue().presentKHR(present) == vk::Result::eSuccess);

    /**
     * Flip-Flop Sync Idx
     */
    syncIdx = 1 - syncIdx;

    /**
     * Debug Metrics
     */
    static uint64_t frames = 0;
    static auto last = std::chrono::high_resolution_clock::now();

    const auto now = _STD chrono::high_resolution_clock::now();
    if (_STD chrono::duration_cast<_STD chrono::milliseconds>(now - last).count() > 1000) {
        _STD cout << "Frames: " << frames << _STD endl;

        frames = 0;
        last = now;
    }

    ++frames;
}

void Graphics::processGraphicPasses() {
    ref<scene::SceneGraph> graph = *scene::Scene::get()->graphs().front();

    constexpr GraphicPassMask masks[] {
        GraphicPassMask::eDepthPass,
        GraphicPassMask::eLightPass,
        GraphicPassMask::eProbePass,
        GraphicPassMask::ePbrPass
    };
    constexpr u8 length = static_cast<u8>(sizeof(masks)) - 1;

    /**
     * Reset synchronization for all processed passes
     */
    [[maybe_unused]] auto resetResult = _device->vkDevice().resetFences(static_cast<u32>(_graphicPassFences.size()),
        _graphicPassFences.data());

    for (u8 idx = 0; idx < length; ++idx) {
        /**
         *
         */
        GraphicPassMask mask = masks[idx];

        /**
         *
         */
        auto& batch = _graphicPassBatches[static_cast<u8>(mask)];
        auto& graphicPass = _graphicPasses[static_cast<u8>(mask)];

        batch.reset();

        /**
         * Wait for Dependencies
         */
        vector<GraphicPassMask> dependencies {};

        if (dependencies.size() >= 1) {
            // TODO: get back dependencies for RenderPass finish
            vector<vk::Fence> waitFences {};

            waitFences.reserve(dependencies.size());
            for (const auto& mask : dependencies) {
                waitFences.push_back(_graphicPassFences[static_cast<u8>(mask)]);
            }

            [[maybe_unused]] auto result = _device->vkDevice().waitForFences(waitFences.size(), waitFences.data(),
                VK_TRUE, UINT64_MAX);
        }

        /**
         * Invoke and Store GraphicsPass
         */
        graphicPass->process(graph, batch);

        /**
         * Submit Batch to Queue
         */
        _graphicsQueue->submit(batch, _graphicPassFences[idx]);
    }

    /**
     * Sync all passes for final computation
     */
    [[maybe_unused]] auto waitAllResult = _device->vkDevice().waitForFences(_graphicPassFences.size(),
        _graphicPassFences.data(), VK_TRUE, UINT64_MAX);
}
