#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::core {
	/**
	 * @details Due to the engine scoped lifecycle, we can reboot the engine a multiple layers,
	 *  therefore the forward stats are defined unique, while backward stats will inherit the underlying value
	 *  of the forward layered stage.
	 */
	enum class EngineState : u8 {
		eUndefined = 0x0,
		ePreInitialized = 0x1,
		eInitialized = 0x2,
		ePostInitialized = 0x3,
		eStarted = 0x4,
		//
		eStopped = ePostInitialized,
		eShutdown = ePreInitialized,
		eExited = eUndefined
	};
}
