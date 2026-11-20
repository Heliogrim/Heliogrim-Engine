#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg {
	class Texture {
	private:
		/**
		 * Internal
		 */
		SharedPtr<void> _internal;

	public:
		/**
		 * Gets the internal state
		 *
		 * @author Julius
		 * @date 07.01.2021
		 *
		 * @returns A cref&lt;managed&lt;void&gt;&gt;
		 */
		[[nodiscard]] cref<SharedPtr<void>> unwrap() const noexcept;
	};
}
