#include "VkCmdMgr.hpp"

#include <Engine.GFX/Command/CommandBatch.hpp>
#include <numeric>

using namespace hg::driver::vk;
using namespace hg;

std::span<const Vector<engine::accel::AccelCommandBuffer>> VkCmdMgr::getCommitted() const noexcept {
    return {
        committed.data(),
        committed.size()
    };
}

size_t VkCmdMgr::getCommitCount() const noexcept {
    return committed.size();
}

size_t VkCmdMgr::getCommittedCount() const noexcept {
    return std::accumulate(
        committed.begin(),
        committed.end(),
        0uLL,
        [](size_t&& acc_, const auto& entry_) {
            return acc_ + entry_.size();
        }
    );
}

std::span<const engine::accel::AccelCommandBuffer> VkCmdMgr::getPending() const noexcept {
    return pending;
}

size_t VkCmdMgr::getPendingCount() const noexcept {
    return pending.size();
}

bool VkCmdMgr::isActiveRoot() const noexcept {
    return active.size() == 1;
}

ref<engine::accel::AccelCommandBuffer> VkCmdMgr::getActive() const noexcept {
    return const_cast<ref<engine::accel::AccelCommandBuffer>>(active.back());
}

ref<engine::accel::AccelCommandBuffer> VkCmdMgr::allocate() {

    const auto pool = queues.front().front()->pool();

    std::scoped_lock<concurrent::UnfairSpinLock> lck { pool->lck() };
    auto result = pool->make();

    active.emplace_back(
        std::move(static_cast<engine::accel::AccelCommandBuffer&&>(result))
    );

    return active.back();
}

void VkCmdMgr::submit() {

    assert(not active.empty());

    auto buffer = std::move(active.back());
    active.pop_back();

    pending.push_back(std::move(buffer));
}

void VkCmdMgr::commit() {

    assert(not pending.empty());

    auto enqueue = std::move(pending);
    const auto queue = queues.front().front();

    const auto batch = engine::gfx::CommandBatch {
        std::move(reinterpret_cast<Vector<engine::gfx::CommandBuffer>&&>(enqueue))
    };
    const auto fence = device->vkDevice().createFence({});

    queue->submit(batch, fence);

    // TODO: committed.push_back(std::move(batch));

    // Warning: Temporary
    [[maybe_unused]] const auto waitResult = device->vkDevice().waitForFences(1, &fence, true, UINT64_MAX);
    device->vkDevice().destroyFence(fence);
}
