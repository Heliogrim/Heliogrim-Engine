#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Variant.hpp>

namespace hg::engine::diag {
	struct NoLocation {};

	/* byte offsets [begin, end); line and column are 1-based and describe `begin` */
	struct SourceSpan {
		String file;
		u32 begin, end;
		u32 line, column;
	};

	struct AssetPath {
		String url;
		String recordPath;
	};

	struct ActorPath {
		u64 actor;
		String componentPath;
	};

	struct ByteOffset {
		String blob;
		u64 offset, length;
	};

	using Location = Variant<NoLocation, SourceSpan, AssetPath, ActorPath, ByteOffset>;
}
