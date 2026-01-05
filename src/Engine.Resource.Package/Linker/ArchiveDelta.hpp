#pragma once

#include <Engine.Common/Variant.hpp>

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
