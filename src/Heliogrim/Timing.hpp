#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg {
	/**
	 * @returns The current delta time in seconds.
	 */
	[[nodiscard]] extern f32 GetDeltaTime() noexcept;
}
