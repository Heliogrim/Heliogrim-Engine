#include "VkNativeBatch.hpp"

#include <Engine.GFX/Command/CommandPool.hpp>
#include <Engine.GFX/Command/CommandQueue.hpp>

#include "VkNativeQueue.hpp"
#include "VkResourceTable.hpp"

using namespace hg::driver::vk;
using namespace hg;

VkNativeBatch::VkNativeBatch() noexcept :
    NativeBatch(),
    _rtable(),
    _batched(),
    _tmpWaits(),
    _tmpWaitFlags(),
    _tmpSignals() {}

VkNativeBatch::~VkNativeBatch() noexcept = default;

bool VkNativeBatch::ready() const noexcept {
    return std::ranges::all_of(
        _batched,
        [](const auto& acb_) {
            return acb_->valid();
        }
    );
}

bool VkNativeBatch::isFaf() const noexcept {
    return std::ranges::any_of(
        _batched,
        [](const auto& acb_) {
            return acb_->isFaf();
        }
    );
}

void VkNativeBatch::commit() {

    InlineAutoArray<ptr<engine::render::cmd::NativeQueue>, 2> queues {};
    enumerateNativeQueues(queues);

    if (queues.empty()) {
        assert(false);
        return;
    }

    /**/

    Vector<::VkCommandBuffer> roots {};
    roots.reserve(_batched.size());

    std::ranges::for_each(
        _batched,
        [&roots](const auto& acb_) {
            if (acb_->root()) {
                roots.push_back(acb_->vkCommandBuffer());
            }

            acb_->commit();
        }
    );

    /**/

    assert(queues.size() == 1uLL);
    auto* const queue = static_cast<ptr<VkNativeQueue>>(queues.front());

    const auto vkDevice = /* ::vk::Device {} */static_cast<ptr<engine::gfx::CommandQueue>>(queue)->device()->vkDevice();
    const auto vkQueue = /* ::VkQueue {} */static_cast<ptr<engine::gfx::CommandQueue>>(queue)->vkQueue();

    const auto fence = vkDevice.createFence({});

    Vector<VkSemaphore> signals {};
    Vector<VkPipelineStageFlags> stages {};

    enumerateReadySignals(signals);
    const auto readyCount = signals.size();

    enumerateWaitSignals(signals, stages);

    const ::VkSubmitInfo si {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        static_cast<u32>(signals.size() - readyCount), signals.data() + readyCount, stages.data(),
        static_cast<u32>(roots.size()), roots.data(),
        static_cast<u32>(readyCount), signals.data()
    };

    const auto result = ::vkQueueSubmit(vkQueue, 1uL, &si, fence);
    assert(result == VK_SUCCESS);

    /* Warning: Dummy */
    // Error (HE-19): When shutdown occurs while current context passed render pass acquisition, this will block "indefinitely"
    [[maybe_unused]] const auto waitResult = vkDevice.waitForFences(1uL, &fence, true, UINT64_MAX);
    vkDevice.destroy(fence);
}

void VkNativeBatch::commitAndDispose() {

    commit();

    /**/

    InlineAutoArray<ptr<engine::gfx::CommandPool>, 2> pools {};
    std::ranges::for_each(
        _batched,
        [&pools](auto&& acb_) {

            auto pool = acb_->pool();
            if (not std::ranges::contains(pools, pool)) {
                pool->lck().acquire();
                pools.emplace_back(pool);
            }

            /**/

            // Warning: Temporay
            acb_->release();
        }
    );

    _batched.clear();

    std::ranges::for_each(
        pools,
        [](auto pool) {
            pool->lck().release();
        }
    );

    /**/

    _tmpWaits.clear();
    _tmpWaitFlags.clear();
    _tmpSignals.clear();
}

bool VkNativeBatch::enumerateNativeQueues(
    ref<InlineAutoArray<ptr<engine::render::cmd::NativeQueue>, 2>> queues_
) const noexcept {

    std::ranges::for_each(
        _batched,
        [&queues_](const auto& acb_) {

            if (not acb_->root()) {
                return;
            }

            auto queue = acb_->pool()->queue();
            if (not std::ranges::contains(queues_, queue)) {
                queues_.emplace_back(queue);
            }

        }
    );

    return not _batched.empty();
}

void VkNativeBatch::add(mref<uptr<engine::accel::AccelCommandBuffer>> cmd_) noexcept {
    _batched.emplace_back(std::move(cmd_));
}

void VkNativeBatch::add(mref<uptr<VkResourceTable>> rt_) noexcept {
    _rtable.emplace_back(std::move(rt_));
}

void VkNativeBatch::enumerateWaitSignals(
    ref<Vector<VkSemaphore>> waitSignals_,
    ref<Vector<VkPipelineStageFlags>> waitStages_
) {
    waitSignals_.insert_range(waitSignals_.end(), _tmpWaits);
    waitStages_.insert_range(waitStages_.end(), _tmpWaitFlags);
}

void VkNativeBatch::enumerateReadySignals(ref<Vector<VkSemaphore>> readySignals_) {
    readySignals_.insert_range(readySignals_.end(), _tmpSignals);
}

ref<decltype(VkNativeBatch::_rtable)> VkNativeBatch::getResourceTables() noexcept {
    return _rtable;
}
