#include "RevDepthPassModelProcessor.hpp"

#if FALSE

#include <Engine.Common/Math/Coordinates.hpp>

#include "../../GraphicPass/GraphicPass.hpp"
#include "../../Scene/SceneNodeModel.hpp"
#include "../../Memory/VkAllocator.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RevDepthPassModelProcessor::RevDepthPassModelProcessor(ptr<const GraphicPass> graphicPass_) :
    GraphicPassModelProcessor(graphicPass_) {}

bool RevDepthPassModelProcessor::operator()(u32 batchIdx_, const ptr<scene::RenderGraph::node_type> node_) noexcept {

    /**
     *
     */
    if (node_->exclusiveSize()/* > 0*/) {

        assert(node_->isLeaf());

        auto** arr { node_->elements() };
        for (auto i = node_->exclusiveSize(); i > 0; --i, ++arr) {
            auto* entry { *arr };
            process(entry->batch(_graphicPass->mask()));
        }
    }

    /**
     * Precheck whether node has state or children
     */
    if (node_->isLeaf()) {
        return false;
    }

    // TODO: Remove!!!
    return !node_->isLeaf();

#if FALSE
    /**
     * Check whether scene node as stored valid element reference; otherwise drop iteration
     */
    auto& stored = node_.payload();
    auto wppl = stored.payload<SceneElement>();

    if (wppl.expired()) {
        return true;
    }

    /**
     *
     */
    auto nse = wppl.lock();
    ref<SceneElement> se = *nse;

    /**
     *
     */
    auto batch = se.batch(_graphicPass->mask());

    /**
     *
     */
    auto state = se.state();

    /**
     *
     */
    _consumed.push_back({});

    /**
     *
     */
    return !node_.isLeaf();
#endif
}

void RevDepthPassModelProcessor::process(mref<ModelBatch> model_) noexcept {

    ProcessedModelBatch pmb {};

    pmb.geometry() = SharedGeometry {
        .vertices = model_.vertices,
        .instances = {},
        .indices = model_.indices
    };

    pmb.executions().push_back(ExecutionRow {
        DistinctBind {},
        DistinctGeometry {
            .transform = model_.transform,
            .instanceCount = 1,
            .instanceOffset = 0,
            .indexCount = model_.indexCount,
            .indexOffset = model_.indexOffset
        }
    });

    _consumed.push_back(_STD move(pmb));
}

void RevDepthPassModelProcessor::postProcess(const ptr<const RenderContext> ctx_) {
    GraphicPassModelProcessor::postProcess(ctx_);

    if (_consumed.empty()) {
        return;
    }

    const auto& device { _graphicPass->device() };

    /**
     * Load
     */
    sptr<Buffer> buffer {};

    const auto it { ctx_->state()->data.find("RevDepthPassModelProcessor::InstanceBuffer"sv) };
    if (it != ctx_->state()->data.end()) {
        const auto stored { _STD static_pointer_cast<Buffer, void>(it->second) };
        buffer = stored;
    }

    if (!buffer || buffer->size < _consumed.size() * sizeof(math::mat4)) {

        /**
         * Allocate Uniform Buffer
         */
        Buffer ib {
            nullptr,
            nullptr,
            device->vkDevice(),
            _consumed.size() * sizeof(math::mat4),
            vk::BufferUsageFlagBits::eStorageBuffer
        };

        const vk::BufferCreateInfo bci {
            vk::BufferCreateFlags {},
            ib.size,
            ib.usageFlags,
            vk::SharingMode::eExclusive,
            0,
            nullptr
        };

        ib.buffer = device->vkDevice().createBuffer(bci);
        assert(ib.buffer);

        ptr<VkAllocator> alloc {
            VkAllocator::makeForBuffer(device, ib.buffer, vk::MemoryPropertyFlagBits::eHostVisible)
        };

        const vk::MemoryRequirements req { device->vkDevice().getBufferMemoryRequirements(ib.buffer) };
        ib.memory = alloc->allocate(req.size);
        ib.bind();

        ib.map();

        delete alloc;

        /**
         * Cleanup & Swap
         */
        if (buffer) {
            buffer->destroy();
        }

        auto nib {
            _STD make_shared<decltype(ib)>(
                _STD move(ib)
            )
        };
        buffer.swap(nib);

        /**
         * Store
         */
        ctx_->state()->data.insert_or_assign("RevDepthPassModelProcessor::InstanceBuffer"sv, buffer);
    }

    /**
     * Update
     */
    auto mem { static_cast<ptr<math::mat4>>(buffer->memory->mapping) };
    for (u64 i = 0; i < _consumed.size(); ++i) {

        auto& entry { _consumed[i] };
        auto translation {
            math::mat4::make_identity().translate(
                entry.executions().front().geometry.transform.position())
        };

        auto scale {
            math::mat4::make_identity().unchecked_scale(entry.executions().front().geometry.transform.scale())
        };

        auto eulerAngles { entry.executions().front().geometry.transform.rotation().euler() };
        auto rotation { math::mat4::make_identity() };
        rotation.rotate(eulerAngles.x, math::vec3_pitch);
        rotation.rotate(eulerAngles.y, math::vec3_yaw);
        rotation.rotate(eulerAngles.z, math::vec3_roll);

        /**
         * Publish Model Matrix
         */
        mem[i] = translation * rotation * scale;

        /**
         * Publish Buffer
         */
        entry.bind().uniform = *buffer;

        /**
         * Publish Meta Data
         */
        entry.executions().front().geometry.instanceCount = 1ui32;
        entry.executions().front().geometry.instanceOffset = static_cast<u32>(i);
    }
}

void RevDepthPassModelProcessor::reset() {
    _consumed.clear();
}


#endif
