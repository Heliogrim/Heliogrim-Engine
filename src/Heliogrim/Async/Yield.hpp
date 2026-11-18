#pragma once

namespace hg {
	/**
	 * Yields the current execution context
	 *
	 * @author Julius
	 * @date 23.10.2021
	 */
	extern void yield();

	/**
	 * Yields when the calling task is in a lower lane and has used its slice
	 */
	extern void checkpoint();
}
