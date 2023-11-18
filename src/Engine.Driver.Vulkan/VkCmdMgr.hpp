#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.GFX.Acc/Command/CommandBuffer.hpp>
#include <Engine.GFX/Device/Device.hpp>

namespace hg::driver::vk {
    enum class VkQueueFeatureSet : u8 {
        eGraphics = 0x1,
        eCompute = 0x2,
        eTransfer = 0x4
    };

    typedef Flag<VkQueueFeatureSet> VkQueueFeatureSetFlag;

    class VkCmdMgr {
    public:
        sptr<engine::gfx::Device> device;

        InlineAutoArray<VkQueueFeatureSetFlag, 3uLL> queueFeatureSets;
        InlineAutoArray<InlineAutoArray<nmpt<engine::gfx::CommandQueue>>, 3uLL> queues;

    public:
        Vector<Vector<engine::gfx::AccelCommandBuffer>> committed;
        Vector<engine::gfx::AccelCommandBuffer> pending;
        InlineAutoArray<engine::gfx::AccelCommandBuffer, 2uLL> active;

    public:
        [[nodiscard]] _STD span<const Vector<engine::gfx::AccelCommandBuffer>> getCommitted() const noexcept;

        [[nodiscard]] size_t getCommitCount() const noexcept;

        [[nodiscard]] size_t getCommittedCount() const noexcept;

        [[nodiscard]] _STD span<const engine::gfx::AccelCommandBuffer> getPending() const noexcept;

        [[nodiscard]] size_t getPendingCount() const noexcept;

    public:
        [[nodiscard]] bool isActiveRoot() const noexcept;

        [[nodiscard]] ref<engine::gfx::AccelCommandBuffer> getActive() const noexcept;

    public:
        ref<engine::gfx::AccelCommandBuffer> allocate();

        void submit();

        void commit();
    };
}
