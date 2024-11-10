#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

#include "ReadonlyLinkedArchive.hpp"

namespace hg::engine::resource {
	class ReadonlyPackageLinker;
}

namespace hg::engine::resource::package {
	class ReadonlyPackageIterator :
		public DenseMap<Guid, ReadonlyLinkedArchive>::const_iterator {
	public:
		friend class ReadonlyPackageLinker;

	public:
		using this_type = ReadonlyPackageIterator;
		using base_type = DenseMap<Guid, ReadonlyLinkedArchive>::const_iterator;

	public:
		using base_type::base_type;

		constexpr explicit ReadonlyPackageIterator(_In_ cref<base_type> base_) noexcept :
			base_type(base_) {}
	};
}
