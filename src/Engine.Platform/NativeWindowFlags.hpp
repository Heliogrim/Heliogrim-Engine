#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::platform {
	enum class NativeWindowFlagBits : u8 {
		eNone = 0x0,
		//
		eFullScreen = 0x1,
		eBorderless = 0x2,
		eShown = 0x4,
		eHidden = 0x8,
		//
		eMovable = 0x10,
		eResizable = 0x20,
		eMinimized = 0x40,
		eMaximized = 0x80
	};

	typedef Flag<NativeWindowFlagBits, u8> NativeWindowFlags;
}
