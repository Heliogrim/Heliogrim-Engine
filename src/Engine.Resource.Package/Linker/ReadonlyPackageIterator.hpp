#pragma once

#include <Engine.Common/Sal.hpp>

#include "ReadonlyLinkedArchive.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
	class ReadonlyPackageLinker;
}

namespace hg::engine::resource::package {
	class ReadonlyPackageIterator :
		public DenseMap<ArchiveGuid, ReadonlyLinkedArchive>::const_iterator {
	public:
		friend class ReadonlyPackageLinker;

	public:
		using this_type = ReadonlyPackageIterator;
		using base_type = DenseMap<ArchiveGuid, ReadonlyLinkedArchive>::const_iterator;

	public:
		using base_type::base_type;

		constexpr explicit ReadonlyPackageIterator(_In_ cref<base_type> base_) noexcept :
			base_type(base_) {}
	};
}
