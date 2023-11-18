#include "VkNativeBatch.hpp"

#include <Engine.GFX/Command/CommandPool.hpp>
#include <Engine.GFX/Command/CommandQueue.hpp>

#include "VkNativeQueue.hpp"

using namespace hg::driver::vk;
using namespace hg;

bool VkNativeBatch::ready() const noexcept {
    return _STD ranges::all_of(
        _batched,
        [](const auto& acb_) {
            return acb_->valid();
        }
    );
}

bool VkNativeBatch::isFaf() const noexcept {
    return _STD ranges::any_of(
        _batched,
        [](const auto& acb_) {
            return acb_->isFaf();
        }
    );
}

void VkNativeBatch::commit() {

    InlineAutoArray<ptr<engine::gfx::render::cmd::NativeQueue>, 2> queues {};
    enumerateNativeQueues(queues);

    if (queues.empty()) {
        assert(false);
        return;
    }

    /**/

    Vector<::VkCommandBuffer> roots {};
    roots.reserve(_batched.size());

    _STD ranges::for_each(
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
    auto queue = static_cast<ptr<VkNativeQueue>>(queues.front());

    const auto vkDevice = /* ::vk::Device {} */static_cast<ptr<engine::gfx::CommandQueue>>(queue)->device()->vkDevice();
    const auto vkQueue = /* ::VkQueue {} */static_cast<ptr<engine::gfx::CommandQueue>>(queue)->vkQueue();

    const auto fence = vkDevice.createFence({});

    Vector<VkSemaphore> signals {};
    Vector<VkPipelineStageFlags> stages {};

    enumerateReadySignals(signals);
    const auto readyCount = signals.size();

    enumerateWaitSignals(signals, stages);

    ::VkSubmitInfo si {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        signals.size() - readyCount, signals.data(), stages.data(),
        static_cast<u32>(roots.size()), roots.data(),
        readyCount, signals.data()
    };

    const auto result = ::vkQueueSubmit(vkQueue, 1uL, &si, fence);
    assert(result == VK_SUCCESS);

    /* Warning: Dummy */
    [[maybe_unused]] const auto waitResult = vkDevice.waitForFences(1uL, &fence, true, UINT64_MAX);
    vkDevice.destroy(fence);
}

void VkNativeBatch::commitAndDispose() {

    commit();

    /**/

    InlineAutoArray<ptr<engine::gfx::CommandPool>, 2> pools {};
    _STD ranges::for_each(
        _batched,
        [&pools](auto&& acb_) {

            auto pool = acb_->pool();
            if (not _STD ranges::contains(pools, pool)) {
                pool->lck().acquire();
                pools.emplace_back(pool);
            }

            /**/

            // Warning: Temporay
            pool->release(*acb_);
        }
    );

    _batched.clear();

    _STD ranges::for_each(
        pools,
        [](auto pool) {
            pool->lck().release();
        }
    );
}

bool VkNativeBatch::enumerateNativeQueues(
    ref<InlineAutoArray<ptr<engine::gfx::render::cmd::NativeQueue>, 2>> queues_
) const noexcept {

    _STD ranges::for_each(
        _batched,
        [&queues_](const auto& acb_) {

            if (not acb_->root()) {
                return;
            }

            auto queue = acb_->pool()->queue();
            if (not _STD ranges::contains(queues_, queue)) {
                queues_.emplace_back(queue);
            }

        }
    );

    return not _batched.empty();
}

void VkNativeBatch::add(mref<uptr<engine::gfx::AccelCommandBuffer>> cmd_) noexcept {
    _batched.emplace_back(_STD move(cmd_));
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
