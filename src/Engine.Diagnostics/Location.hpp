#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
