#include "VkScopedCmdMgr.hpp"

using namespace hg::driver::vk;
using namespace hg;

u16 VkScopedCmdMgr::scopeId() const noexcept {
    return _scopeId;
}

tl::optional<engine::gfx::AccelCommandBuffer&> VkScopedCmdMgr::active() const noexcept {
    return _cmdBuf.map_or(
        [](const auto& cmd_) {
            return tl::make_optional<ref<engine::gfx::AccelCommandBuffer>>(
                const_cast<ref<engine::gfx::AccelCommandBuffer>>(cmd_)
            );
        },
        tl::make_optional<ref<engine::gfx::AccelCommandBuffer>>(tl::nullopt)
    );
}
