#pragma once

#include <Engine.Common/Sal.hpp>

#include "__fwd.hpp"
#include "../Structure/RootScopedSlot.hpp"
#include "../Structure/__fwd.hpp"

namespace hg::engine::serialization {
	class StructuredArchive {
	public:
		using this_type = StructuredArchive;

	public:
		StructuredArchive(_Inout_ ref<resource::Archive> archive_);

		StructuredArchive(cref<this_type>) = delete;

		StructuredArchive(mref<this_type>) noexcept = delete;

		~StructuredArchive() = default;

	private:
		nmpt<resource::Archive> _archive;

	public:
		[[nodiscard]] nmpt<resource::Archive> getArchive() const noexcept;

	public:
		[[nodiscard]] RootScopedSlot getRootSlot() const;

		[[nodiscard]] RootScopedSlot insertRootSlot();
	};
}
