#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
	inline constexpr Array<_::byte, 6uLL> PackageMagicBytes = {
		_::byte { 0x49 }, _::byte { 0x4D }, _::byte { 0x50 }, _::byte { 0x41 }, _::byte { 0x43 }, _::byte { 0x4B }
	};
	inline constexpr u8 PackageMagicVersion[1] = { 0x01 };
}
