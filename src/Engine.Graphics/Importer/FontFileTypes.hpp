#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource;
import std;
#endif

namespace hg::engine::graphics {
	namespace FontFileType {
		extern const res::FileTypeId Ttf;
		extern const res::FileTypeId Otf;

		[[nodiscard]] extern std::span<const res::FileTypeId> asSpan() noexcept;
	}
}
