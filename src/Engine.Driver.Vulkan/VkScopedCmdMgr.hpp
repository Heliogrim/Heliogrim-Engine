#pragma once

#include <tl/optional.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.GFX.Acc/Command/CommandBuffer.hpp>

namespace hg::driver::vk {
    class VkScopedCmdMgr {
    public:
        constexpr VkScopedCmdMgr() noexcept = default;

        constexpr VkScopedCmdMgr(u16 scopeId_, ref<engine::gfx::AccelCommandBuffer> cmd_) noexcept :
            _scopeId(scopeId_),
            _cmdBuf(cmd_) {}

        constexpr ~VkScopedCmdMgr() noexcept = default;

    private:
        u16 _scopeId;
        tl::optional<engine::gfx::AccelCommandBuffer&> _cmdBuf;

    public:
        [[nodiscard]] u16 scopeId() const noexcept;

        [[nodiscard]] tl::optional<engine::gfx::AccelCommandBuffer&> active() const noexcept;
    };
}
