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
		ReadWriteLinkedArchive(auto&&... args_) requires std::is_constructible_v<ReadonlyLinkedArchive, decltype(args_)&&...> :
			ReadonlyLinkedArchive { ::hg::forward<decltype(args_)>(args_)... },
			changes() {}

		ReadWriteLinkedArchive(const this_type&) = delete;

		ReadWriteLinkedArchive(this_type&&) noexcept = default;

	public:
		this_type& operator=(const this_type&) = delete;

		this_type& operator=(this_type&&) noexcept = default;

	public:
		Vector<ArchiveDelta> changes;
	};
}
