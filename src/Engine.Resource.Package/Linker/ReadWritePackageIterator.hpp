#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/StableUnorderedMap.hpp>

#include "ReadWriteLinkedArchive.hpp"

namespace hg::engine::resource {
	class ReadWritePackageLinker;
}

namespace hg::engine::resource::package {
	class ReadWritePackageIterator :
		public StableUnorderedMap<ArchiveGuid, ReadWriteLinkedArchive>::const_iterator {
	public:
		friend class ReadWritePackageLinker;

	public:
		using this_type = ReadWritePackageIterator;
		using base_type = StableUnorderedMap<ArchiveGuid, ReadWriteLinkedArchive>::const_iterator;

	public:
		using base_type::base_type;

		constexpr explicit ReadWritePackageIterator(_In_ cref<base_type> base_) noexcept :
			base_type(base_) {}
	};

	class MutableReadWritePackageIterator :
		public StableUnorderedMap<ArchiveGuid, ReadWriteLinkedArchive>::iterator {
	public:
		friend class ReadWritePackageLinker;

	public:
		using this_type = ReadWritePackageIterator;
		using base_type = StableUnorderedMap<ArchiveGuid, ReadWriteLinkedArchive>::iterator;

	public:
		using base_type::base_type;

		constexpr explicit MutableReadWritePackageIterator(_In_ cref<base_type> base_) noexcept :
			base_type(base_) {}
	};
}
