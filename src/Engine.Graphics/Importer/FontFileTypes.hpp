#pragma once

#include <span>
#include <Engine.Resource/FileTypeId.hpp>

namespace hg::engine::gfx {
	namespace FontFileType {
		extern const res::FileTypeId Ttf;
		extern const res::FileTypeId Otf;

		[[nodiscard]] extern std::span<const res::FileTypeId> asSpan() noexcept;
	}
}
