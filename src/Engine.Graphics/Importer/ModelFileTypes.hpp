#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource;
import std;
#endif

namespace hg::engine::graphics {
	namespace ModelFileType {
		extern const res::FileTypeId Fbx;
		extern const res::FileTypeId Obj;

		[[nodiscard]] extern std::span<const res::FileTypeId> asSpan() noexcept;
	}
}
