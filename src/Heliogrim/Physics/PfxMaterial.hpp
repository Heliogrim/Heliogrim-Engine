#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg {
	class PfxMaterial {
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
