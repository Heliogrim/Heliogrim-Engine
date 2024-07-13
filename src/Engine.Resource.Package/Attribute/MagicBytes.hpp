#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/Array.hpp>

namespace hg::engine::resource {
	constexpr static inline Array<_::byte, 6uLL> PackageMagicBytes = {
		_::byte { 0x49 }, _::byte { 0x4D }, _::byte { 0x50 }, _::byte { 0x41 }, _::byte { 0x43 }, _::byte { 0x4B }
	};
	constexpr static inline u8 PackageMagicVersion[1] = { 0x01 };
}
