#include "RootScopedSlot.hpp"

#include <Engine.Common/Make.hpp>
#include "RootSlot.hpp"

using namespace hg::engine::serialization;
using namespace hg;

RootScopedSlot::RootScopedSlot(_Inout_ ref<resource::Archive> archive_, const ScopedSlotStateFlags flags_) :
	RecordScopedSlot(ScopedSlotState { flags_ }, RootSlot::makeRootState(archive_)) {}
