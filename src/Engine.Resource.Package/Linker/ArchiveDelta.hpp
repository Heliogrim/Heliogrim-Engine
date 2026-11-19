#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
	class Archive;
}

namespace hg::engine::resource::package {
	/**/
	struct ArchiveDeltaAdd {
		streamoff where;
		/**/
		Archive& sizedData;
	};

	struct ArchiveDeltaDrop {
		streamoff where;
		streamsize byteCount;
	};

	struct ArchiveDeltaReplace {
		streamoff where;
		streamsize byteCount;
		/**/
		Archive& withSizedData;
	};

	/**/

	using ArchiveDelta = Variant<ArchiveDeltaAdd, ArchiveDeltaDrop, ArchiveDeltaReplace>;
}
