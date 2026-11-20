#pragma once
#include <Engine.Accel.Command/CommandBuffer.hpp>
#include <Engine.Graphics/Device/Device.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::driver::vk {
    enum class VkQueueFeatureSet : u8 {
        eGraphics = 0x1,
        eCompute = 0x2,
        eTransfer = 0x4
    };

    typedef Flag<VkQueueFeatureSet> VkQueueFeatureSetFlag;

    class VkCmdMgr {
    public:
        sptr<engine::graphics::Device> device;

        InlineAutoArray<VkQueueFeatureSetFlag, 3uLL> queueFeatureSets;
        InlineAutoArray<InlineAutoArray<nmpt<engine::graphics::CommandQueue>>, 3uLL> queues;

    public:
        Vector<Vector<engine::accel::AccelCommandBuffer>> committed;
        Vector<engine::accel::AccelCommandBuffer> pending;
        InlineAutoArray<engine::accel::AccelCommandBuffer, 2uLL> active;

    public:
        [[nodiscard]] std::span<const Vector<engine::accel::AccelCommandBuffer>> getCommitted() const noexcept;

        [[nodiscard]] size_t getCommitCount() const noexcept;

        [[nodiscard]] size_t getCommittedCount() const noexcept;

        [[nodiscard]] std::span<const engine::accel::AccelCommandBuffer> getPending() const noexcept;

        [[nodiscard]] size_t getPendingCount() const noexcept;

    public:
        [[nodiscard]] bool isActiveRoot() const noexcept;

        [[nodiscard]] ref<engine::accel::AccelCommandBuffer> getActive() const noexcept;

    public:
        ref<engine::accel::AccelCommandBuffer> allocate();

        void submit();

        void commit();
    };
}
