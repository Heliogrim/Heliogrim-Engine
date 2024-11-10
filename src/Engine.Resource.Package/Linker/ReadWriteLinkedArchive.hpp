#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "ArchiveDelta.hpp"
#include "ReadonlyLinkedArchive.hpp"

namespace hg::engine::resource::package {
	struct ReadWriteLinkedArchive :
		public ReadonlyLinkedArchive {
	public:
		using this_type = ReadWriteLinkedArchive;

	public:
		Vector<ArchiveDelta> changes;
	};
}
