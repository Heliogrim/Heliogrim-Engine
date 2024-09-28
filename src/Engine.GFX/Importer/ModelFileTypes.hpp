#pragma once

#include <span>
#include <Engine.Resource/FileTypeId.hpp>

namespace hg::engine::gfx {
	namespace ModelFileType {
		extern const res::FileTypeId Fbx;
		extern const res::FileTypeId Obj;

		[[nodiscard]] extern std::span<const res::FileTypeId> asSpan() noexcept;
	}
}
