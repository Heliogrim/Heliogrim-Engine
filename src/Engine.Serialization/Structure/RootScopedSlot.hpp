#pragma once

#include "RecordScopedSlot.hpp"
#include "../Archive/__fwd.hpp"

namespace hg::engine::serialization {
	class RootScopedSlot final :
		public RecordScopedSlot {
	public:
		friend class ::hg::engine::serialization::StructuredArchive;

	public:
		using this_type = RootScopedSlot;

	public:
		RootScopedSlot(_Inout_ ref<resource::Archive> archive_, const ScopedSlotStateFlags flags_);
	};
}
