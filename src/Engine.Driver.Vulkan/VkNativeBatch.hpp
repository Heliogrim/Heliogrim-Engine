#pragma once
#include <Engine.Accel.Command/CommandBuffer.hpp>
#include <Engine.GFX.Render.Command/NativeBatch.hpp>

namespace hg::driver::vk {
    class VkNativeBatch final :
        public engine::gfx::render::cmd::NativeBatch {
    public:
        using this_type = VkNativeBatch;

    public:
        constexpr VkNativeBatch() noexcept = default;

        constexpr ~VkNativeBatch() noexcept override = default;

    private:
        InlineAutoArray<uptr<engine::accel::AccelCommandBuffer>, 2uLL> _batched;

    public:
        [[nodiscard]] bool ready() const noexcept override;

        [[nodiscard]] bool isFaf() const noexcept override;

    public:
        void commit() override;

        void commitAndDispose() override;

    public:
        bool enumerateNativeQueues(
            ref<InlineAutoArray<ptr<engine::gfx::render::cmd::NativeQueue>, 2>> queues_
        ) const noexcept override;

    public:
        void add(mref<uptr<engine::accel::AccelCommandBuffer>> cmd_) noexcept;

    private:
        void enumerateWaitSignals(
            _Out_ ref<Vector<VkSemaphore>> waitSignals_,
            _Out_ ref<Vector<VkPipelineStageFlags>> waitStages_
        );

        void enumerateReadySignals(
            _Out_ ref<Vector<VkSemaphore>> readySignals_
        );

        /**/
    public:
        Vector<VkSemaphore> _tmpWaits;
        Vector<VkPipelineStageFlags> _tmpWaitFlags;
        Vector<VkSemaphore> _tmpSignals;
    };
}
