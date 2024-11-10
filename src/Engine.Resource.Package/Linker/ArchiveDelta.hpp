#pragma once

#include <Engine.Common/Variant.hpp>

namespace hg::engine::resource {
	class Archive;
}

namespace hg::engine::resource::package {
	/**/
	struct ArchiveDeltaAdd {
		streamoff where;
		streamsize size;
		/**/
		Archive& data;
	};

	struct ArchiveDeltaDrop {
		streamoff where;
		streamsize size;
	};

	struct ArchiveDeltaReplace {
		streamoff where;
		streamsize size;
		/**/
		Archive& data;
	};

	/**/

	using ArchiveDelta = Variant<ArchiveDeltaAdd, ArchiveDeltaDrop, ArchiveDeltaReplace>;
}
