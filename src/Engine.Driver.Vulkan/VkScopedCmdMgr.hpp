#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Command;
import Engine.Common;
#endif

namespace hg::driver::vk {
	class VkScopedCmdMgr {
	public:
		constexpr VkScopedCmdMgr() noexcept = default;

		constexpr VkScopedCmdMgr(u16 scopeId_, ref<engine::accel::AccelCommandBuffer> cmd_) noexcept :
			_scopeId(scopeId_),
			_cmdBuf(cmd_) {}

		VkScopedCmdMgr(cref<VkScopedCmdMgr>) = delete;

		constexpr VkScopedCmdMgr(mref<VkScopedCmdMgr> other_) noexcept = default;

		constexpr ~VkScopedCmdMgr() noexcept = default;

	private:
		u16 _scopeId;
		tl::optional<engine::accel::AccelCommandBuffer&> _cmdBuf;

	public:
		[[nodiscard]] u16 scopeId() const noexcept;

		[[nodiscard]] tl::optional<engine::accel::AccelCommandBuffer&> active() const noexcept;
	};
}
