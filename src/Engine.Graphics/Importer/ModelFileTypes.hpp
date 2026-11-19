#pragma once

#include <Engine.Resource/FileTypeId.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg::engine::graphics {
	namespace ModelFileType {
		extern const res::FileTypeId Fbx;
		extern const res::FileTypeId Obj;

		[[nodiscard]] extern std::span<const res::FileTypeId> asSpan() noexcept;
	}
}
