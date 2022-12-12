#include "UiMainStageNode.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#ifdef _DEBUG
#include <cassert>
#endif

#include <Engine.GFX/Renderer/HORenderPass.hpp>
#include <Engine.GFX/Renderer/RenderPassState.hpp>
#include <Engine.GFX/Renderer/RenderStagePass.hpp>
#include <Engine.GFX/Renderer/RenderDataToken.hpp>
#include <Engine.Logging/Logger.hpp>

#include "Engine.GFX/VkFixedPipeline.hpp"
#include "Engine.GFX/API/VkTranslate.hpp"
#include "Engine.GFX/Buffer/Buffer.hpp"
#include "Engine.GFX/Command/CommandBuffer.hpp"
#include "Engine.GFX/Shader/DiscreteBindingPool.hpp"
#include "Engine.GFX/Shader/Factory.hpp"
#include "Engine.GFX/Shader/PrototypeBinding.hpp"
#include "Engine.GFX/Shader/ShaderStorage.hpp"
#include "Engine.GFX/Texture/TextureFactory.hpp"

using namespace ember::engine::gfx::glow::ui::render;
using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

#define IMAGE_DESCRIPTOR_INDEX 0

#if TRUE
#include "Engine.Reflow/Widget/Panel.hpp"
#include "Engine.Reflow/Window/Window.hpp"
#include "../TestUI.hpp"
#include <Engine.Input/MouseButtonEvent.hpp>
#include <Engine.Input/MouseMoveEvent.hpp>
#include <Engine.Input/MouseWheelEvent.hpp>
#include <Engine.Input/DragDropEvent.hpp>
#include <Engine.Reflow/Command/ReflowCommandBuffer.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>
#include <Engine.Reflow/Style/StyleKeyStack.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <mutex>
#include <atomic>

static sptr<engine::reflow::Window> uiTestPanel {};
_STD mutex uiTestMtx = _STD mutex {};
#endif

struct UiTransformBlock {
    math::vec2 translate;
    math::vec2 scale;
};

UiMainStageNode::UiMainStageNode() :
    RenderStageNode() {}

UiMainStageNode::~UiMainStageNode() = default;

void UiMainStageNode::setup(cref<sptr<Device>> device_) {
    SCOPED_STOPWATCH

    /**
     * Store device
     */
    _device = device_;

    /**
     *
     */
    setupLORenderPass();

    /**
     *
     */
    setupShader();

    /**
     *
     */
    setupPipeline();

    /**
     *
     */
    setupDefaultImage();
}

void UiMainStageNode::destroy() {
    SCOPED_STOPWATCH

    /**
     *
     */
    if (_pipeline) {
        _pipeline->destroy();
        _pipeline.reset();
    }

    /**
     *
     */
    for (auto& entry : _requiredDescriptorPools) {
        delete[] entry.pPoolSizes;

        entry.pPoolSizes = nullptr;
        entry.poolSizeCount = 0ui32;
    }

    /**
     *
     */
    for (auto& entry : _requiredBindingGroups) {
        _device->vkDevice().destroyDescriptorSetLayout(entry.vkSetLayout());
    }

    /**
     *
     */
    destroyLORenderPass();

    /**
     *
     */
    destroyDefaultImage();

    #if TRUE
    if (uiTestPanel) {
        uiTestPanel.reset();
    }
    #endif
}

bool UiMainStageNode::allocate(const ptr<HORenderPass> renderPass_) {
    const auto state { renderPass_->state() };

    /**
     * Allocate Command Buffer
     */
    auto cmd { _device->graphicsQueue()->pool()->make() };

    /**
     * Allocate Framebuffer
     */
    auto framebuffer { allocateFramebuffer(renderPass_) };

    /**
     * Store State
     */
    state->data.insert_or_assign("UiMainStageNode::CommandBuffer"sv, make_sptr<decltype(cmd)>(_STD move(cmd)));
    state->data.insert_or_assign("UiMainStageNode::Framebuffer"sv,
        make_sptr<decltype(framebuffer)>(_STD move(framebuffer)));

    state->data.insert_or_assign("UiMainStageNode::ImageDescriptorPool"sv,
        make_sptr<vk::DescriptorPool>(VK_NULL_HANDLE));
    state->data.insert_or_assign("UiMainStageNode::ImageDescriptors"sv,
        make_sptr<Vector<shader::DiscreteBindingGroup>>());

    return true;
}

bool UiMainStageNode::free(const ptr<HORenderPass> renderPass_) {
    const auto state { renderPass_->state() };

    /**
     *
     */
    auto it { state->data.find("UiMainStageNode::CommandBuffer"sv) };
    if (it != state->data.end()) {
        auto cmd { _STD static_pointer_cast<CommandBuffer, void>(it->second) };

        if (cmd->vkCommandBuffer()) {
            auto& lck { cmd->lck() };
            lck.acquire();
            cmd->release();
            lck.release();
        }

        state->data.erase(it);
    }

    /**
     *
     */
    it = state->data.find("UiMainStageNode::Framebuffer"sv);
    if (it != state->data.end()) {
        auto framebuffer { _STD static_pointer_cast<Framebuffer, void>(it->second) };

        freeFramebuffer(_STD move(*framebuffer));
        state->data.erase(it);
    }

    /**
     *
     */
    it = state->data.find("UiMainStageNode::VertexBuffer"sv);
    if (it != state->data.end()) {
        auto buffer { _STD static_pointer_cast<Buffer, void>(it->second) };
        buffer->destroy();
        state->data.erase(it);
    }

    it = state->data.find("UiMainStageNode::IndexBuffer"sv);
    if (it != state->data.end()) {
        auto buffer { _STD static_pointer_cast<Buffer, void>(it->second) };
        buffer->destroy();
        state->data.erase(it);
    }

    const auto idpit { state->data.find("UiMainStageNode::ImageDescriptorPool"sv) };
    const auto idit { state->data.find("UiMainStageNode::ImageDescriptors"sv) };
    if (idpit != state->data.end()) {
        auto pool { _STD static_pointer_cast<vk::DescriptorPool, void>(idpit->second) };
        auto groups { _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(idit->second) };

        // Warning: Do not free descriptor sets individually when not using free-able pools
        /*
        for (const auto& group : *groups) {
            _device->vkDevice().freeDescriptorSets(*pool, 1ui32, &group.vkSet());
        }
         */

        _device->vkDevice().destroyDescriptorPool(*pool);

        state->data.erase(idpit);
        state->data.erase(idit);
    }

    return true;
}

Vector<RenderDataToken> UiMainStageNode::requiredToken() noexcept {
    return {};
}

Vector<RenderDataToken> UiMainStageNode::optionalToken() noexcept {
    return {};
}

void UiMainStageNode::before(const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_) const {
    #if TRUE
    if (!uiTestPanel) {
        reflow::WindowManager::make();

        testLoad(_device);
        uiTestPanel = buildTestUI(_device);

        auto& emitter { engine::Engine::getEngine()->getEmitter() };
        emitter.on<input::event::MouseButtonEvent>([](cref<input::event::MouseButtonEvent> event_) {
            _STD unique_lock<_STD mutex> lck { uiTestMtx };
            reflow::WindowManager::get()->dispatch(uiTestPanel, event_);
        });
        emitter.on<input::event::MouseMoveEvent>([](cref<input::event::MouseMoveEvent> event_) {
            _STD unique_lock<_STD mutex> lck { uiTestMtx };

            reflow::WindowManager::get()->dispatch(uiTestPanel, event_);
        });
        emitter.on<input::event::MouseWheelEvent>([](cref<input::event::MouseWheelEvent> event_) {
            _STD unique_lock<_STD mutex> lck { uiTestMtx };
            reflow::WindowManager::get()->dispatch(uiTestPanel, event_);
        });
        emitter.on<input::event::DragDropEvent>([](cref<input::event::DragDropEvent> event_) {
            _STD unique_lock<_STD mutex> lck { uiTestMtx };
            reflow::WindowManager::get()->dispatch(uiTestPanel, event_);
        });
        emitter.on<input::event::KeyboardEvent>([](cref<input::event::KeyboardEvent> event_) {
            _STD unique_lock<_STD mutex> lck { uiTestMtx };
            reflow::WindowManager::get()->dispatch(uiTestPanel, event_);
        });
    }
    #endif

    RenderStageNode::before(renderPass_, stagePass_);
}

void UiMainStageNode::invoke(const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_,
    const non_owning_rptr<SceneNodeModel> model_) const {
    if (!uiTestPanel) {
        return;
    }

    auto* state { renderPass_->state().get() };
    auto& data { state->data };

    /**
     *
     */
    const auto cmdEntry { data.at("UiMainStageNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    /**
     *
     */
    const auto frameEntry { data.at("UiMainStageNode::Framebuffer"sv) };
    auto& framebuffer { *_STD static_pointer_cast<Framebuffer, void>(frameEntry) };

    /**
     *
     */
    sptr<Buffer> vertexBuffer {};
    sptr<Buffer> indexBuffer {};

    sptr<Vector<shader::DiscreteBindingGroup>> imageDescriptors {};
    sptr<vk::DescriptorPool> imageDescriptorPool {};

    const auto vit { data.find("UiMainStageNode::VertexBuffer"sv) };
    const auto iit { data.find("UiMainStageNode::IndexBuffer"sv) };

    const auto idpit { data.find("UiMainStageNode::ImageDescriptorPool"sv) };
    const auto idit { data.find("UiMainStageNode::ImageDescriptors"sv) };

    if (vit != data.end()) {
        vertexBuffer = _STD static_pointer_cast<Buffer, void>(vit->second);
    } else {
        vertexBuffer = make_sptr<Buffer>();
    }

    if (iit != data.end()) {
        indexBuffer = _STD static_pointer_cast<Buffer, void>(iit->second);
    } else {
        indexBuffer = make_sptr<Buffer>();
    }

    /**
     *
     */
    reflow::ReflowCommandBuffer uiCmd {};
    uiCmd._runningIndexes.reserve(32ui64 * 1024ui64);
    uiCmd._runningVertices.reserve(32ui64 * 1024ui64);

    math::vec2 ava {
        static_cast<float>(renderPass_->target()->width()),
        static_cast<float>(renderPass_->target()->height())
    };
    math::vec2 one { 1.F };
    math::vec2 zero { 0.F };
    reflow::FlowContext context {
        math::fExtent2D { ava.x, ava.y, 0.F, 0.F },
        math::fExtent2D { ava.x, ava.y, 0.F, 0.F },
    };

    _STD unique_lock<_STD mutex> lck { uiTestMtx };
    reflow::StyleKeyStack stack {};

    auto start = _STD chrono::high_resolution_clock::now();
    uiTestPanel->flow(context, ava, ava, stack);
    auto end = _STD chrono::high_resolution_clock::now();

    //IM_DEBUG_LOGF("Flow took: {}", _STD chrono::duration_cast<_STD chrono::microseconds>(end - start).count());

    start = _STD chrono::high_resolution_clock::now();
    uiTestPanel->shift(context, zero);
    end = _STD chrono::high_resolution_clock::now();

    //IM_DEBUG_LOGF("Shift took: {}", _STD chrono::duration_cast<_STD chrono::microseconds>(end - start).count());

    start = _STD chrono::high_resolution_clock::now();
    math::fExtent2D rootScissor { context.scissor };
    uiCmd.pushScissor(rootScissor);
    uiTestPanel->render(&uiCmd);
    assert(rootScissor == uiCmd.popScissor());
    end = _STD chrono::high_resolution_clock::now();

    //IM_DEBUG_LOGF("Render took: {}", _STD chrono::duration_cast<_STD chrono::microseconds>(end - start).count());

    lck.unlock();

    /**
     *
     */
    auto& rv { uiCmd._runningVertices };
    auto& ri { uiCmd._runningIndexes };

    /**
     *
     */
    const u64 vertexSize { sizeof(uivertex) * rv.size() };
    const u64 indexSize { sizeof(u32) * ri.size() };
    const u64 imageCount { uiCmd._images.size() + /* Default Image*/ 1 };

    /**/
    if (vertexSize == 0 || indexSize == 0) {
        return;
    }
    /**/

    if (vertexBuffer->size != vertexSize) {
        vertexBuffer->destroy();

        auto& buffer { *vertexBuffer };
        buffer.size = vertexSize;
        buffer.usageFlags = vk::BufferUsageFlagBits::eVertexBuffer;
        buffer.device = _device->vkDevice();

        const vk::BufferCreateInfo info {
            vk::BufferCreateFlags {},
            buffer.size,
            buffer.usageFlags,
            vk::SharingMode::eExclusive,
            0,
            nullptr
        };

        buffer.buffer = _device->vkDevice().createBuffer(info);
        assert(buffer.buffer);

        [[maybe_unused]] auto result {
            memory::allocate(&state->alloc, _device, buffer.buffer,
                MemoryProperty::eHostVisible | MemoryProperty::eHostCoherent, buffer.memory)
        };
        buffer.bind();
    }

    vertexBuffer->write<uivertex>(uiCmd._runningVertices.data(), uiCmd._runningVertices.size());

    if (indexBuffer->size != indexSize) {
        indexBuffer->destroy();

        auto& buffer { *indexBuffer };
        buffer.size = indexSize;
        buffer.usageFlags = vk::BufferUsageFlagBits::eIndexBuffer;
        buffer.device = _device->vkDevice();

        const vk::BufferCreateInfo info {
            vk::BufferCreateFlags {},
            buffer.size,
            buffer.usageFlags,
            vk::SharingMode::eExclusive,
            0,
            nullptr
        };

        buffer.buffer = _device->vkDevice().createBuffer(info);
        assert(buffer.buffer);

        [[maybe_unused]] auto result {
            memory::allocate(&state->alloc, _device, buffer.buffer,
                MemoryProperty::eHostVisible | MemoryProperty::eHostCoherent, buffer.memory)
        };
        buffer.bind();
    }

    indexBuffer->write<u32>(uiCmd._runningIndexes.data(), uiCmd._runningIndexes.size());

    /**
     *
     */
    if (idpit != data.end()) {
        imageDescriptorPool = _STD static_pointer_cast<vk::DescriptorPool, void>(idpit->second);
        imageDescriptors = _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(idit->second);
    }

    if (imageDescriptors->size() < imageCount) {
        rebuildImageDescriptors(imageDescriptorPool, imageDescriptors, imageCount);

        /**/
        (*imageDescriptors)[0].getById(1).store(_defaultImage);
        /**/

        state->data.insert_or_assign("UiMainStageNode::ImageDescriptorPool"sv, imageDescriptorPool);
        state->data.insert_or_assign("UiMainStageNode::ImageDescriptors"sv, imageDescriptors);
    }

    /**
     * Store State
     */
    state->data.insert_or_assign("UiMainStageNode::VertexBuffer"sv, vertexBuffer);
    state->data.insert_or_assign("UiMainStageNode::IndexBuffer"sv, indexBuffer);

    /**
     *
     */
    cmd.reset();
    cmd.begin();

    cmd.beginRenderPass(*_loRenderPass, framebuffer);

    cmd.bindPipeline(_pipeline.get(), Viewport {
        renderPass_->target()->width(),
        renderPass_->target()->height(),
        0.F,
        1.F
    });

    /**
     *
     */
    UiTransformBlock consts {
        math::vec2 { -1.F },
        math::vec2 { 2.F / ava.x, 2.F / ava.y }
    };

    cmd.vkCommandBuffer().pushConstants(
        static_cast<ptr<VkFixedPipeline>>(_pipeline.get())->vkLayout(),
        vk::ShaderStageFlagBits::eVertex,
        0, sizeof(UiTransformBlock),
        &consts
    );

    /**
     * Dispatch
     */
    cmd.bindVertexBuffer(0, *vertexBuffer, 0);
    cmd.bindIndexBuffer(*static_cast<ptr<IndexBuffer>>(indexBuffer.get()), 0);

    cmd.bindDescriptor(0, imageDescriptors->at(0).vkSet());
    //cmd.drawIndexed(1, 0, static_cast<u32>(uiCmd._runningIndexes.size()), 0, 0);

    /**/
    u32 firstIndices { static_cast<u32>(uiCmd._runningIndexes.size()) };
    if (not uiCmd._scissorIndices.empty() && uiCmd._scissorIndices.front().first < firstIndices) {
        firstIndices = uiCmd._scissorIndices.front().first;
    }
    if (not uiCmd._imageIndices.empty() && uiCmd._imageIndices.front().first < firstIndices) {
        firstIndices = uiCmd._imageIndices.front().first;
    }

    /**/
    u32 lastStartIndices { static_cast<u32>(uiCmd._runningIndexes.size()) };
    if (not uiCmd._scissorIndices.empty() && uiCmd._scissorIndices.back().second < lastStartIndices) {
        lastStartIndices = uiCmd._scissorIndices.back().second;
    }
    if (not uiCmd._imageIndices.empty() && uiCmd._imageIndices.back().second < lastStartIndices) {
        lastStartIndices = uiCmd._imageIndices.back().second;
    }

    /**/
    if (uiCmd._scissorIndices.empty()) {
        uiCmd._scissorIndices.push_back(_STD make_pair(~0ui32, ~0ui32));
    }

    if (uiCmd._imageIndices.empty()) {
        uiCmd._imageIndices.push_back(_STD make_pair(~0ui32, ~0ui32));
    }

    /**/
    auto sciIdxIt { 0ui32 };
    auto imgIdxIt { 0ui32 };

    vk::Rect2D vkScissor {
        vk::Offset2D { static_cast<s32>(rootScissor.offsetX), static_cast<s32>(rootScissor.offsetY) },
        vk::Extent2D { static_cast<u32>(rootScissor.width), static_cast<u32>(rootScissor.height) }
    };

    /**/
    cmd.vkCommandBuffer().setScissor(0, 1, &vkScissor);
    cmd.drawIndexed(1, 0, firstIndices, 0, 0);

    /**/
    u32 idx { firstIndices };
    while (idx < lastStartIndices) {
        const auto sciIdx { idx >= uiCmd._scissorIndices[sciIdxIt].second ? ++sciIdxIt : sciIdxIt };
        const auto imgIdx { idx >= uiCmd._imageIndices[imgIdxIt].second ? ++imgIdxIt : imgIdxIt };

        auto nextStride { MIN(uiCmd._scissorIndices[sciIdx].second, uiCmd._imageIndices[imgIdx].second) };

        if (uiCmd._scissorIndices[sciIdx].first > idx && uiCmd._scissorIndices[sciIdx].first < nextStride) {
            nextStride = uiCmd._scissorIndices[sciIdx].first;
        }
        if (uiCmd._imageIndices[imgIdx].first > idx && uiCmd._imageIndices[imgIdx].first < nextStride) {
            nextStride = uiCmd._imageIndices[imgIdx].first;
        }

        /*
        if (prevBaseIdx < ppi.first) {
            cmd.bindDescriptor(0, imageDescriptors->at(0).vkSet());
            cmd.drawIndexed(1, 0, ppi.first - prevBaseIdx, prevBaseIdx, 0);
        }
         */

        /**/
        const auto& uiScissor { uiCmd._scissors[sciIdx] };
        vkScissor.offset.x = static_cast<s32>(uiScissor.offsetX);
        vkScissor.offset.y = static_cast<s32>(uiScissor.offsetY);
        vkScissor.extent.width = static_cast<u32>(uiScissor.width);
        vkScissor.extent.height = static_cast<u32>(uiScissor.height);
        cmd.vkCommandBuffer().setScissor(0, 1, &vkScissor);

        /**/
        if (uiCmd._imageIndices[imgIdxIt].first <= idx && idx < uiCmd._imageIndices[imgIdxIt].second) {
            auto& dbind { (*imageDescriptors)[imgIdx + 1] };
            //dbind.getById(1).store(*uiCmd._images[idx]);
            if (uiCmd._images[imgIdx].is<Texture>()) {
                dbind.getById(1).storeAs(*uiCmd._images[imgIdx].as<Texture>(), vk::ImageLayout::eShaderReadOnlyOptimal);
            } else if (uiCmd._images[imgIdx].is<VirtualTexture>()) {
                const auto* view { uiCmd._images[imgIdx].as<VirtualTexture>() };
                dbind.getById(1).store(view);
            } else if (uiCmd._images[imgIdx].is<VirtualTextureView>()) {
                const auto* view { uiCmd._images[imgIdx].as<VirtualTextureView>() };
                dbind.getById(1).store(view->owner());
            }

            cmd.bindDescriptor(0, dbind.vkSet());
        } else {
            cmd.bindDescriptor(0, imageDescriptors->at(0).vkSet());
        }

        /**/
        cmd.drawIndexed(1, 0, nextStride - idx, idx, 0);

        /**/
        idx = nextStride;
    }

    if (lastStartIndices < uiCmd._runningIndexes.size()) {
        vkScissor.offset.x = static_cast<s32>(rootScissor.offsetX);
        vkScissor.offset.y = static_cast<s32>(rootScissor.offsetY);
        vkScissor.extent.width = static_cast<u32>(rootScissor.width);
        vkScissor.extent.height = static_cast<u32>(rootScissor.height);

        cmd.vkCommandBuffer().setScissor(0, 1, &vkScissor);
        cmd.bindDescriptor(0, imageDescriptors->at(0).vkSet());
        cmd.drawIndexed(1, 0, uiCmd._runningIndexes.size() - lastStartIndices, lastStartIndices, 0);
    }

    /**
     *
     */
    cmd.endRenderPass();
    cmd.end();

    // Warning: Temporary Solution
    for (const auto& wait : uiCmd._imageWait) {
        stagePass_->batch().pushBarrier(wait, vk::PipelineStageFlagBits::eColorAttachmentOutput);
    }
    for (const auto& signal : uiCmd._imageSignal) {
        stagePass_->batch().pushSignal(signal);
    }

    /**
     * Submit Command Buffer to Command Batch
     */
    stagePass_->batch().push(cmd);
}

void UiMainStageNode::after(const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_) const {
    RenderStageNode::after(renderPass_, stagePass_);
}

void UiMainStageNode::setupShader() {
    /**
     *
     */
    shader::Factory shaderFactory { _device };
    ShaderStorage& shaderStorage { ShaderStorage::get() };

    /**
     * Prepare Prototype Bindings
     */
    shader::PrototypeBinding textureAtlas {
        shader::BindingType::eImageSampler,
        1ui32,
        shader::BindingUpdateInterval::ePerInstance,
        "uiPassTextureAtlas"
    };

    /**
     * Prepare Prototype Shader
     */
    shader::Prototype vertexPrototype { shader::ShaderType::eVertex, "uiMainPass" };

    auto vertexShaderCode { read_shader_file("resources/shader/uipass.vert.spv") };
    vertexPrototype.storeCode(vertexShaderCode.data(), vertexShaderCode.size());

    shader::Prototype fragmentPrototype { shader::ShaderType::eFragment, "uiMainPass" };

    auto fragmentShaderCode { read_shader_file("resources/shader/uipass.frag.spv") };
    fragmentPrototype.storeCode(fragmentShaderCode.data(), fragmentShaderCode.size());
    fragmentPrototype.add(textureAtlas);

    /**
     * Build Shader and Bindings
     */
    auto factoryResult {
        shaderFactory.build({
            vertexPrototype,
            fragmentPrototype
        })
    };

    /**
     * Store Shader to Storage
     */
    for (const auto& entry : factoryResult.shaders) {
        shaderStorage.store(entry);
    }

    /**
     * Prepare required Descriptor Pools
     */
    for (const auto& group : factoryResult.groups) {
        Vector<vk::DescriptorPoolSize> sizes {};
        for (const auto& binding : group.shaderBindings()) {
            auto it {
                _STD find_if(sizes.begin(), sizes.end(),
                    [type = binding.type()](cref<vk::DescriptorPoolSize> entry_) {
                        return entry_.type == api::vkTranslateBindingType(type);
                    })
            };

            if (it == sizes.end()) {
                sizes.push_back({ api::vkTranslateBindingType(binding.type()), 1 });
                continue;
            }

            ++(it->descriptorCount);
        }

        // Persist data structure
        auto* sizeMem { new vk::DescriptorPoolSize[sizes.size()] };
        for (u32 i = 0; i < sizes.size(); ++i) {
            sizeMem[i] = _STD move(sizes[i]);
        }

        const vk::DescriptorPoolCreateInfo dpci {
            vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
            1ui32,
            static_cast<u32>(sizes.size()),
            sizeMem
        };

        _requiredDescriptorPools.push_back(dpci);
        _requiredBindingGroups.push_back(group);
    }
}

void UiMainStageNode::setupPipeline() {
    assert(!_pipeline);

    _pipeline = make_sptr<VkFixedPipeline>(_device, _loRenderPass);
    assert(_pipeline);

    _pipeline->topology() = PrimitiveTopology::eTriangleList;
    _pipeline->viewport() = Viewport {};

    _pipeline->inputs().push_back(FixedPipelineInput {
        0ui8,
        InputRate::ePerVertex,
        sizeof(uivertex),
        Vector<InputAttribute> {
            { 0ui32, TextureFormat::eR32G32B32Sfloat, static_cast<u32>(offsetof(uivertex, position)) },
            { 1ui32, TextureFormat::eR8G8B8A8Unorm, static_cast<u32>(offsetof(uivertex, color)) },
            { 2ui32, TextureFormat::eR32G32B32Sfloat, static_cast<u32>(offsetof(uivertex, uvm)) }
        }
    });

    _pipeline->vertexStage().shaderSlot().name() = "uiMainPass";
    _pipeline->fragmentStage().shaderSlot().name() = "uiMainPass";

    _pipeline->rasterizationStage().cullFace() = RasterCullFace::eNone;
    // Warning: Unsure whether we should drop depth check support cause of ui component override
    _pipeline->rasterizationStage().depthCheck() = false;

    auto& blend { static_cast<ptr<VkFixedPipeline>>(_pipeline.get())->blending() };
    const vk::PipelineColorBlendAttachmentState colorState {
        VK_TRUE,
        vk::BlendFactor::eSrcAlpha,
        vk::BlendFactor::eOneMinusSrcAlpha,
        vk::BlendOp::eAdd,
        vk::BlendFactor::eZero,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
    };

    // Push color blending for output color
    blend.push_back(colorState);

    static_cast<ptr<VkFixedPipeline>>(_pipeline.get())->pushConstants().push_back({
        vk::ShaderStageFlagBits::eVertex,
        0ui32,
        sizeof(UiTransformBlock)
    });

    /**
     *
     */
    _pipeline->setup();
}

void UiMainStageNode::setupLORenderPass() {
    /**
     *
     */
    _loRenderPass = make_sptr<pipeline::LORenderPass>(_device);

    // Color Attachment :: Used to store composed color
    _loRenderPass->set(0, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        vk::Format::eB8G8R8A8Unorm,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR
    });

    /**
     *
     */
    _loRenderPass->setup();
}

void UiMainStageNode::destroyLORenderPass() {
    if (_loRenderPass) {
        _loRenderPass->destroy();
        _loRenderPass.reset();
    }
}

Framebuffer UiMainStageNode::allocateFramebuffer(const non_owning_rptr<HORenderPass> renderPass_) {
    /**
     * Create Framebuffer
     */
    const auto* factory { TextureFactory::get() };

    Framebuffer buffer { _device };
    buffer.setExtent(renderPass_->target()->extent());
    buffer.setRenderPass(_loRenderPass);

    /**
     * Create Framebuffer :: Attach Target
     */
    auto target { renderPass_->target() };
    if (!target->vkView()) {
        factory->buildView(*target);
    }

    buffer.add({ target });

    /**
     *
     */
    buffer.setup();
    return buffer;
}

void UiMainStageNode::freeFramebuffer(mref<Framebuffer> framebuffer_) {
    auto forwarded { _STD move(framebuffer_) };
    forwarded.destroy();
}

void UiMainStageNode::rebuildImageDescriptors(
    ref<sptr<vk::DescriptorPool>> pool_,
    ref<sptr<Vector<shader::DiscreteBindingGroup>>> groups_,
    u32 count_
) const {
    const auto* prevPool { pool_.get() };
    // Warning: Do not free descriptor sets individually when not using free-able pools
    /*
    for (const auto& group : *groups_) {
        _device->vkDevice().freeDescriptorSets(*prevPool, 1ui32, &group.vkSet());
    }
     */

    _device->vkDevice().destroyDescriptorPool(*prevPool);

    auto* groups { groups_.get() };
    groups->clear();
    groups->shrink_to_fit();

    pool_.reset();

    /**
     *
     */

    auto poolReq { _requiredDescriptorPools[IMAGE_DESCRIPTOR_INDEX] };

    poolReq.maxSets *= count_;
    Vector<vk::DescriptorPoolSize> sizes {};
    for (u32 i { 0ui32 }; i < poolReq.poolSizeCount; ++i) {
        sizes.push_back(poolReq.pPoolSizes[i]);
        sizes.back().descriptorCount *= count_;
    }
    poolReq.setPPoolSizes(sizes.data());
    poolReq.setPoolSizeCount(sizes.size());

    auto pool { _device->vkDevice().createDescriptorPool(poolReq) };

    const auto& setReq { _requiredBindingGroups[IMAGE_DESCRIPTOR_INDEX] };

    Vector<vk::DescriptorSetLayout> layouts {};
    layouts.resize(count_);
    _STD fill_n(layouts.begin(), count_, setReq.vkSetLayout());
    auto sets { _device->vkDevice().allocateDescriptorSets({ pool, count_, layouts.data() }) };

    pool_ = make_sptr<vk::DescriptorPool>(_STD move(pool));

    groups->reserve(sets.size());

    for (const auto& set : sets) {
        groups->push_back(setReq.useDiscrete(set));
    }
}

#if TRUE
#include <Engine.GFX/Loader/RevTextureLoader.hpp>
#include <Engine.GFX/Graphics.hpp>
#endif

void UiMainStageNode::setupDefaultImage() {
    engine::gfx::RevTextureLoader loader { engine::Engine::getEngine()->getGraphics()->cacheCtrl() };
    _defaultImage = loader.__tmp__load({
        ""sv,
        R"(R:\\Development\C++\Vulkan API\Game\resources\imports\ktx\default_ui.ktx)"
    });

    Vector<vk::ImageMemoryBarrier> imgBarriers {};
    imgBarriers.push_back({
        vk::AccessFlags {},
        vk::AccessFlagBits::eShaderRead,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        _defaultImage.buffer().image(),
        vk::ImageSubresourceRange {
            vk::ImageAspectFlagBits::eColor,
            0,
            _defaultImage.mipLevels(),
            0,
            _defaultImage.layer()
        }
    });

    auto pool = _device->graphicsQueue()->pool();
    pool->lck().acquire();
    engine::gfx::CommandBuffer iiCmd = pool->make();
    iiCmd.begin();

    /**
     * Transform
     */
    iiCmd.vkCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlagBits::eAllCommands, vk::DependencyFlags {},
        0, nullptr,
        0, nullptr,
        static_cast<uint32_t>(imgBarriers.size()), imgBarriers.data()
    );

    iiCmd.end();
    iiCmd.submitWait();
    iiCmd.release();

    pool->lck().release();

    _defaultImage.buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    engine::gfx::TextureFactory::get()->buildView(_defaultImage);
}

void UiMainStageNode::destroyDefaultImage() {
    _defaultImage.destroy();
}
